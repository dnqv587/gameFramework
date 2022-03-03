#include <iostream>
#include <string>

#include "libuv.h"
#include "../../3rd/http_parser/http_parser.h"
//#include "../../../safeTransmission/Hash.h"
//#include "../../../safeTransmission/Base64.h"

static uv_loop_t* event_loop = NULL;//事件循环对象

static uv_tcp_t tcp_server;//tcp句柄

struct ws_context//数据状态
{
	int is_shake_hand;//是否已经握手
	char* data;//读取数据的buffer
};

static void tcp_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	ws_context* wc = (ws_context*)handle->data;

	if (wc->data != NULL)//释放上次读到的数据
	{
		free(wc->data);
		wc->data = nullptr;
	}
	wc->data = (char*)malloc(suggested_size + 1);//suggested_size建议的大小；+1放‘\0’
	buf->base = (char*)wc->data;
	buf->len = suggested_size;
}

static void close_cb(uv_handle_t* handle)
{
	ws_context* wc = (ws_context*)handle->data;
	if (wc->data)
	{
		free(wc->data);//释放最后一次读到的数据
		wc->data = nullptr;
	}
	free(handle);
	std::cout << "close client" << std::endl;
}

static void shutdown_cb(uv_shutdown_t* req, int status)
{

	uv_close((uv_handle_t*)req->handle, close_cb);//关闭套接字
	free(req);
}

static void tcp_write_cb(uv_write_t* req, int status)
{
	if (status == 0)
		std::cout << "send success" << std::endl;

	free(req);
}

static void send_data(uv_stream_t* stream, char* send_data, int send_len)
{
	//写数据
	uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
	uv_buf_t send_buf = uv_buf_init(send_data, send_len);
	uv_write(req, stream, &send_buf, 1, tcp_write_cb);
}


static char file_sec_key[512];
static char value_sec_key[512];
static int is_sec_key = 0;//是否解析到了Sec-WebSocket-Key

static char web_migic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//base64(sha1(key+web_migic))
static char web_respond[] = "TTTP/1.1 101 Switching Protoclos \r\n"
"Upgrade:websocket\r\n"
"Connection:Upgrade\r\n"
"Sec-WebSocket-Accept:%s\r\n"
"WebSocket-Protocol:chat\r\n\r\n";

static int header_field_cb(http_parser* p, const char* at, size_t length)
{
	if (strncmp(at, "Sec-WebSocket-Key", length) == 0)
	{
		is_sec_key = 1;//解析到了
	}
	else
	{
		is_sec_key = 0;//没解析到
	}
	return 0;
}

static int header_value_cb(http_parser* p, const char* at, size_t length)
{
	if (!is_sec_key)//没解析到Sec-WebSocket-Key
	{
		return 0;
	}

	strncpy(value_sec_key, at, length);
	value_sec_key[length] = 0;

	if (is_sec_key)
	{
		std::cout << value_sec_key << std::endl;

	}

	return 0;
}

//解析报文--握手
static void ws_connect_shake_hand(uv_stream_t* stream, char* data, unsigned long len)
{
	/*
	http_parser_settings settings;
	http_parser_settings_init(&settings);
	settings.on_header_field = header_field_cb;
	settings.on_header_value = header_value_cb;

	http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);
	http_parser_execute(&parser, &settings, data, len);

	//key+migic
	static char key_migic[512];
	sprintf(key_migic, "%s%s", value_sec_key, web_migic);

	std::cout << key_migic << std::endl;

	Hash hash(Hash_SHA1);
	hash.updateData(key_migic);
	Base64 bs64;
	std::string base_buf = bs64.encode(hash.finalData());

	std::cout << base_buf << std::endl;
	static char send_buf[512];
	sprintf(send_buf, web_respond, base_buf.c_str());

	std::cout << send_buf << std::endl;

	send_data(stream, send_buf, strlen(send_buf));
	*/
}

static void ws_on_recv_data(uv_stream_t* stream, unsigned char* data, unsigned int len)
{
	if (data[0] != 0x81 && data[0] != 0x82)
	{
		return;
	}

	unsigned int data_len = data[1]&0x0000007f;
	int head_size = 2;
	if (data_len >= 126)//后面两个字节表示数据长度
	{
		data_len = data[3] | (data[2] << 8);
		head_size += 2;
	}
	else if (data_len == 127)//后面8个字节表示数据长度
	{
		unsigned char netLen[8];
		memcpy(netLen, (void*)data[2], 8);
		unsigned int low = ntohs((u_short)netLen);
	}

	unsigned char* mask = data + head_size;//掩码
	unsigned char* body = data + head_size + 4;//数据部分

	for (int i = 0; i < data_len; ++i)//遍历后面所有的数据
	{
		body[i] = body[i] ^ mask[i % 4];
	}

	static char test_buf[4096];
	memcpy(test_buf, body, data_len);
	test_buf[data_len] = 0;
	std::cout << test_buf << std::endl;

}
/*
说明：读事件回调
参数：
	1：读事件句柄
	2：读到了多少字节的数据
	3：数据所在的buf
*/
static void tcp_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	//连接断开
	if (nread < 0)
	{
		uv_shutdown_t* req = (uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
		memset(req, 0x00, sizeof(uv_shutdown_t));
		uv_shutdown(req, stream, shutdown_cb);//断开连接
		return;
	}

	buf->base[nread] = 0;

	//std::cout << "recv" << nread << ":" << buf->base << std::endl;

	//解析websocket
	std::cout << "start websocket" << std::endl;
	ws_context* wc = (ws_context*)stream->data;
	//如果没有握手，则进入websoket握手协议
	if (!wc->is_shake_hand)
	{
		ws_connect_shake_hand(stream, buf->base, buf->len);//解析报文
		wc->is_shake_hand = 1;
		return;
	}


	//如果握手成功，则进入收数据协议

	if (buf->base[0]==0x88)//客户端关闭
	{
		std::cout << "web close" << std::endl;
		return;
	}

	//ws的正常数据，暂时不处理粘包等问题
	//假设一次性都可以收完发过来得数据包
	ws_on_recv_data(stream, (unsigned char*)buf->base, nread);
}

static void connection_cb(uv_stream_t* server, int status)
{
	std::cout << "new client coming" << std::endl;

	//接入客户端
	uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	memset(client, 0x00, sizeof(uv_tcp_t));
	uv_tcp_init(event_loop, client);
	uv_accept(server, (uv_stream_t*)client);

	//告诉event_loop,让他管理某个事件

	/*
	说明：管理读事件
	参数：
		1：对象
		2：申请读入数据的内存
	*/
	struct ws_context* wc = (ws_context*)malloc(sizeof(ws_context));
	memset(wc, 0x00, sizeof(ws_context));
	client->data = wc;

	uv_read_start((uv_stream_t*)client, tcp_alloc_cb, tcp_read_cb);
}

void websocket()
{
	event_loop = uv_default_loop();//创建默认的事件loop

	memset(&tcp_server, 0x00, sizeof(uv_tcp_t));

	uv_tcp_init(event_loop, &tcp_server);//初始化udp

	//绑定端口
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", 8080, &addr);
	int ret = uv_tcp_bind(&tcp_server, (sockaddr*)&addr, 0);
	if (ret != 0)
	{
		goto FAILED;
	}

	/*
	参数：
		1:对象
		2：监听的数目，SOMAXCONN：最大数目
	*/
	uv_listen((uv_stream_t*)&tcp_server, SOMAXCONN, connection_cb);

	uv_run(event_loop, UV_RUN_DEFAULT);//开启事件循环等待事件的发生

FAILED:
	std::cout << "end" << std::endl;

}