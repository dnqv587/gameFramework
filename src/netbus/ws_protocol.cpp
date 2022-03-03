#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string.h>

#include "ws_protocol.h"
#include "session.h"
#include "../../3rd/http_parser/http_parser.h"
#include "../crypto/Base64.h"
#include "../crypto/Sha1.h"
#include "../utils/cache_alloc.h"
#include "session_uv.h"

#ifdef WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif


static char file_sec_key[512];
static char value_sec_key[512];
static int is_sec_key = 0;//是否解析到了Sec-WebSocket-Key
static int has_sec_key = 0;//是否有Key
static int is_shaker_ended = 0;//websocket握手是否结束

static char web_migic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
//base64(sha1(key+web_migic))
static char web_respond[] = "HTTP/1.1 101 Switching Protocols\r\n"
"Upgrade:websocket\r\n"
"Connection: Upgrade\r\n"
"Sec-WebSocket-Accept: %s\r\n"
"WebSocket-Protocol:chat\r\n\r\n";

extern "C"
{

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
		has_sec_key = 1;

		return 0;
	}

	static int message_end_cb(http_parser*)
	{
		is_shaker_ended = 1;

		return 0;
	}
}
 
bool ws_protocol::ws_shake_hand(session* s, char* body, int len)
{
	http_parser_settings settings;
	http_parser_settings_init(&settings);
	settings.on_header_field = header_field_cb;
	settings.on_header_value = header_value_cb;
	settings.on_message_complete = message_end_cb;

	http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);
	is_sec_key = 0;
	is_shaker_ended = 0;
	has_sec_key = 0;
	http_parser_execute(&parser, &settings, body, len);

	//key+migic
	if (has_sec_key && is_shaker_ended)//存在key并且握手数据包收完了了
	{
		static char key_migic[512];
		sprintf(key_migic, "%s%s", value_sec_key, web_migic);

		//std::cout << key_migic << std::endl;

		Sha1 hash;
		hash.Update(key_migic);
		Base64 bs64;
		std::string base_buf = bs64.encode(hash.Final());

		//std::cout << base_buf << std::endl;
		static char send_buf[512];
		sprintf(send_buf, web_respond, base_buf.c_str());

		//std::cout << send_buf << std::endl;

		s->send_data((unsigned char*)send_buf, strlen(send_buf));

		return true;
	}
	return false;
}

bool ws_protocol::read_ws_header(unsigned char* recv_data, int recv_len, int* pkg_size, int* out_header_size)
{
	if (recv_data[0] != 0x81 && recv_data[0] != 0x82)
	{
		return false;
	}
	if (recv_len < 2)//长度问题
	{
		return false;
	}
	unsigned int data_len = recv_data[1] & 0x0000007f;
	int head_size = 2;
	if (data_len >= 126)//后面两个字节表示数据长度
	{
		head_size += 2;
		if (recv_len < head_size)//长度不够
		{
			return false;
		}
		data_len = recv_data[3] | (recv_data[2] << 8);
		
	}
	else if (data_len == 127)//后面8个字节表示数据长度
	{
		head_size += 8;
		if (recv_len < head_size)//长度不够
		{
			return false;
		}
		unsigned char netLen[8];
		memcpy(netLen, (void*)recv_data[2], 8);
		data_len = ntohs((u_short)netLen);
		
	}

	head_size += 4;//4个mask
	*pkg_size = data_len + head_size;
	*out_header_size = head_size;

	return true;

}

void ws_protocol::parser_ws_recv_data(unsigned char* raw_data, unsigned char* mask, int raw_len)
{
	for (int i = 0; i < raw_len; ++i) 
	{
		raw_data[i] = raw_data[i] ^ mask[i % 4];
	}
}

unsigned char* ws_protocol::package_ws_send_data(const unsigned char* raw_data, int len, int* ws_data_len)
{
	int head_size = 2;
	if (len > 125 && len < 65536) {
		head_size += 2;
	}
	else if (len >= 65536) {
		head_size += 8;
		return NULL;
	}

	// cache malloc
	//unsigned char* data_buf = (unsigned char*)malloc(head_size + len);
	unsigned char* data_buf = (unsigned char*)cache_alloc(session_uv::wbuf_allocer, head_size + len);

	data_buf[0] = 0x81;//0x81字符串，0x82 arraybuf
	if (len <= 125) {
		data_buf[1] = len;
	}
	else if (len > 125 && len < 65536) {
		data_buf[1] = 126;
		data_buf[2] = (len & 0x0000ff00) >> 8;
		data_buf[3] = (len & 0x000000ff);
	}

	memcpy(data_buf + head_size, raw_data, len);
	*ws_data_len = (head_size + len);

	return data_buf;

}

void ws_protocol::free_ws_send_pkg(unsigned char* ws_pkg)
{
	//free(ws_pkg);
	cache_free(session_uv::wbuf_allocer, ws_pkg);
}
