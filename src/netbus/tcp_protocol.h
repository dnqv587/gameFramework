#pragma once
#include "../utils/cache_alloc.h"
//TCP协议模块

class tcp_protocol
{
public:
	//读取包头
	/*
	data:数据
	data_len：数据长度
	pkg_size：读取的包长--传出
	out_header_size：包头的长度
	*/
	static bool read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size);
	//打包数据
	static unsigned char* package(const unsigned char* raw_data, int len, int* pkg_len);
	//释放数据内存
	static void release_package(unsigned char* tp_pkg);
};