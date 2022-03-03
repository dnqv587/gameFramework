#pragma once


#include "http.h"

static char str1[4096];

//配置回调函数
static int message_begin(http_parser* p)
{
	std::cout << "message_begin" << std::endl;

	return 0;
}

static int message_complete(http_parser* p)
{
	std::cout << "message_complete" << std::endl;

	return 0;
}

/*
at:解析到的起始地址
length：解析到的长度
*/
static int url_cb(http_parser*, const char* at, size_t length)
{
	std::string url_buf(at, length);
	std::cout << url_buf << std::endl;

	return 0;
}

static int body_cb(http_parser*, const char* at, size_t length)
{
	std::string body_buf(at, length);
	std::cout << body_buf << std::endl;

	return 0;
}

static int header_cb(http_parser*, const char* at, size_t length)
{
	std::string header_buf(at, length);
	std::cout << header_buf << std::endl;

	return 0;
}

static int value_cb(http_parser*, const char* at, size_t length)
{
	std::string value_buf(at, length);
	std::cout << value_buf << std::endl;

	return 0;
}

static http_parser_settings settings = 
{
	.on_message_begin = message_begin,
	.on_url = url_cb,
	.on_header_field = header_cb,
	.on_header_value = value_cb,
	.on_body= body_cb,
	.on_message_complete = message_complete,
};

static http_parser parser;//定义对象

void http()
{
	memset(str1, 0, 4096);
	strcat(str1, "POST http://demo.git.com/sum.php HTTP/1.1\r\n");
	strcat(str1, "Host: demo.git.com\r\n");
	strcat(str1, "Content-Length: 65\r\n");
	strcat(str1, "Content-Type: application/x-www-form-urlencoded\r\n");
	strcat(str1, "\r\n");
	strcat(str1, "mathod=adb_signe&token=0E1FEECD0EE54E3B8568A536A7036D78B1AC7EEE");
	strcat(str1, "\r\n\r\n");

	/*
	函数说明：初始化
	参数说明：
		1：对象
		2：消息类型
			HTTP_REQUEST：请求消息
			HTTP_RESPONSE：响应消息
			HTTP_BOTH：两者
	*/
	http_parser_init(&parser,HTTP_REQUEST);

	/*
	函数说明：配置参数
	参数说明：
		1：对象
		2：配置回调函数
		3:要解析的文本
		4:长度
	*/
	http_parser_execute(&parser, &settings, str1, strlen(str1));
	


}




