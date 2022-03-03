#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include<fcntl.h>
#include <direct.h>


#include "libuv.h"


static uv_loop_t* event_loop = NULL;//事件循环对象
static uv_udp_t server;//UDP的句柄
static uv_tcp_t tcp_server;//tcp句柄
static uv_timer_t timer;//timer句柄


//-------------------------------------------TCP-----------------------------------------------
/*
uv_stream_t数据结构：
struct uv_stream_s {
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
};

uv_tcp_t数据结构：
struct uv_tcp_s {
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
  UV_TCP_PRIVATE_FIELDS
};

uv_handle_t数据结构
struct uv_handle_s {
  UV_HANDLE_FIELDS
};

强转梯度---继承关系
uv_handle_t > uv_stream_t > uv_tcp_t
*/

/*
说明：申请读入数据的内存
参数：
	1：发生读事件的句柄
	2：建议分配多大的内存，来保存数据
	3：准备好的内存，通过uv_buf_t，告诉event_loop申请的内存地址
*/
static void tcp_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	if (handle->data != NULL)//释放上次读到的数据
	{
		free(handle->data);
		handle->data = nullptr;
	}
	handle->data = malloc(suggested_size + 1);//suggested_size建议的大小；+1放‘\0’
	buf->base = (char*)handle->data;
	buf->len = suggested_size;
}

static void close_cb(uv_handle_t* handle)
{
	if (handle->data)
	{
		free(handle->data);//释放最后一次读到的数据
		handle->data = nullptr;
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
	if(status==0)
		std::cout << "send success" << std::endl;

	free(req);
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

	std::cout << "recv" << nread << ":" << buf->base << std::endl;

	//写数据
	uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
	uv_buf_t send_buf= uv_buf_init((char*)"send data", sizeof("send data"));
	uv_write(req, stream, &send_buf, 1, tcp_write_cb);
}

static void connection_cb(uv_stream_t* server, int status)
{
	std::cout << "new client coming" << std::endl;

	//接入客户端
	uv_tcp_t* client =(uv_tcp_t*) malloc(sizeof(uv_tcp_t));
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
	uv_read_start((uv_stream_t*)client, tcp_alloc_cb, tcp_read_cb);
}

void TCP()
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


//-----------------------------------------UDP--------------------------------------------------
//分配回调函数
static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	if (handle->data != NULL)
	{
		free(handle->data);
		handle->data = NULL;
	}
	handle->data = malloc(suggested_size + 1);//suggested_size建议的大小；+1放‘\0’
	buf->base = (char*)handle->data;
	buf->len = suggested_size;
}


//send处理回调函数
static void udp_send_cb(uv_udp_send_t* req, int status)
{
	if (status == 0)//发送成功
	{
		std::cout << "send success" << std::endl;
	}
	free(req);

}

//recv处理回调函数
static void udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
	char ip[128];
	uv_ip4_name((sockaddr_in*)addr, ip, sizeof(ip));
	int port = ntohs(((sockaddr_in*)addr)->sin_port);
	printf("ip:%s:%d nread = % d\n", ip, port, nread);
	char* str_buf = (char*)handle->data;
	str_buf[nread] = '\0';
	std::cout << "recv:" << str_buf << std::endl;


	uv_buf_t w_buf;

	w_buf = uv_buf_init((char*)"send data", sizeof("send data"));
	//写数据
	uv_udp_send_t* req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
	uv_udp_send(req, handle, &w_buf, 1, addr, udp_send_cb);

}



void  UDP()
{
	event_loop = uv_default_loop();//创建默认的事件loop
	memset(&server, 0x00, sizeof(uv_udp_t));

	uv_udp_init(event_loop, &server);//初始化udp

	//绑定端口
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", 8080, &addr);
	uv_udp_bind(&server, (sockaddr*)&addr, 0);

	//告诉时间循环，要管理的事件
	uv_udp_recv_start(&server, alloc_cb, udp_recv_cb);

	

	uv_run(event_loop, UV_RUN_DEFAULT);//开启事件循环等待事件的发生

}

//-----------------Timer-----------
static void timerTest_cb(uv_timer_t* handle)
{
	std::cout << "timer called" << std::endl;
	//uv_timer_stop(handle); //停止timer句柄
}

void timer_test()
{
	event_loop = uv_default_loop();//创建默认的事件loop

	uv_timer_init(event_loop, &timer);//初始化timer句柄

	uv_timer_start(&timer, timerTest_cb, 5000, 0);//timeout:第一次间隔运行时间；repeat：之后间隔的运行时间--0为只运行一次

	uv_run(event_loop, UV_RUN_DEFAULT);

}


//自定义timer

struct Timer
{
	uv_timer_t uv_timer;//libuv处理句柄
	void (*timer_cb)(void* udata);
	void* udata;
	int repeat_count;//-1一直循环
};

//构建Timer结构体
Timer* alloc_timer(void (*timer_cb)(void* udata), void* udata, int repeat_count)
{
	Timer* t = (Timer*)malloc(sizeof(Timer));
	memset(t, 0x00, sizeof(Timer));

	t->timer_cb = timer_cb;
	t->udata = udata;
	t->repeat_count = repeat_count;

	uv_timer_init(uv_default_loop(), &t->uv_timer); //初始化句柄

	return t;
}

void free_timer(Timer* t)
{
	free(t);
}

void timer_cb(uv_timer_t* handle)
{
	Timer* t = (Timer*)handle->data;
	if (t->repeat_count > 0)
	{
		--t->repeat_count;
		t->timer_cb(t->udata);//递归
	}
	if (t->repeat_count == 0)
	{
		uv_timer_stop(&t->uv_timer); //停止timer句柄
		free_timer(t);//释放
	}

	t->timer_cb(t->udata);//repeat_count为-1，一直执行
}

/*
参数：
void (*timer_cb)(void* udata)：回调函数，当timer出发时回调
void* udata：用户传的自定义的数据结构，回调函数执行的时候的udata就是这个udata
float after_sec：多少秒开始执行
int repeat_count：执行多少次，-1代表一直执行
返回:timer的句柄
*/
//构建timer
Timer* schedule(void (*timer_cb)(void* udata), void* udata,int after_sec,int repeat_count)
{
	Timer* t = alloc_timer(timer_cb, udata, repeat_count);

	//libuv启动timer
	t->uv_timer.data = t;//存储Timer数据
	uv_timer_start(&t->uv_timer, (uv_timer_cb)timer_cb, after_sec , after_sec );

	return t;
}


//回调函数只执行一次
Timer* schedule_once(void (*timer_cb)(void* udata), void* udata, int after_sec)
{
	return schedule(timer_cb, udata, after_sec, 1);
}

//取消timer
void  cancel_timer(Timer* t)
{
	if (t->repeat_count == 0)//全部触发完成
	{
		return;
	}
	uv_timer_stop(&t->uv_timer);
	free(t);
}

void callback_timer(void* data)
{
	char* str = (char*)data;
	std::cout << str << std::endl;
}

void timer_indi()
{
	event_loop = uv_default_loop();//创建默认的事件loop

	Timer* t = schedule(callback_timer, (char*)"Hello", 1000, 5);


	uv_run(event_loop, UV_RUN_DEFAULT);
}

//-------------------------------------------异步文件读写-----------------------------------------------


static uv_fs_t req;

//uv_file :文件句柄对象，打开文件后的handle
static uv_file fs_handle;


static char buf[1024];

//读取文件后的回调函数
void read_fs_cb(uv_fs_t* req)
{
	
	std::cout << "read :" << req->result << std::endl;

	buf[req->result] = 0;//字符串文件结尾
	std::cout << buf << std::endl;

	uv_fs_req_cleanup(req);

	uv_fs_close(event_loop, req, fs_handle, NULL);
	uv_fs_req_cleanup(req);
}

//打开文件后的回调函数
void open_fs_cb(uv_fs_t* req)
{
	//result:每次请求的结果都是这个值来返回；打开文件---result返回打开文件句柄对象uv_file
		//读文件：result读到数据的长度
		//写文件：result为写入数据的长度
	fs_handle = req->result;

	uv_fs_req_cleanup(req);//释放这个请求req所占的资源

	std::cout << "open success" << std::endl;

	//关闭文件句柄
	//uv_fs_close(event_loop, req, fs_handle, NULL);
	//uv_fs_req_cleanup(req);

	//读取文件
	
	uv_buf_t mem_buf = uv_buf_init(buf, 1024);
	uv_fs_read(event_loop, req, fs_handle, &mem_buf, 1, 0, read_fs_cb);

}

void file()
{
	event_loop = uv_default_loop();

	//打开文件
	/*
	loop：事件循环
	uv_fs_t:req请求对象
	path：文件路径
	flags：标志--一般为0
	mode：可读、可写...；需要#include<fcntl.h>
	cb:回调函数
	*/
	std::string path = getcwd(NULL, 0);
	path += "/test.txt";

	uv_fs_open(event_loop, &req, path.c_str(), 0, O_RDONLY, open_fs_cb);


	uv_run(event_loop, UV_RUN_DEFAULT);
}

//---------------------------------------异步标准输入输出---------------------------------

static uv_fs_t w_req;
void std_out_in()
{
	event_loop = uv_default_loop();

	uv_buf_t w_buf = uv_buf_init((char*)"Good by\n", 8);
	uv_fs_write(event_loop, &w_req, (uv_file)1, &w_buf, 1, 0, NULL);//stdin---0;stdout---1
	uv_fs_req_cleanup(&w_req);//不需要回调函数可以清空


	uv_run(event_loop, UV_RUN_DEFAULT);
}