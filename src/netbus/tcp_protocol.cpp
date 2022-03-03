#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>

#include "tcp_protocol.h"
#include "session_uv.h"


bool tcp_protocol::read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size)
{
	if (data_len < 2)//长度小于包头长度
	{
		std::cout << "recved data length is too short" << std::endl;
		return false;
	}

	*pkg_size = data[0] | (data[1] << 8);
	*out_header_size = 2;
	return true;
}

unsigned char* tcp_protocol::package(const unsigned char* raw_data, int len, int* pkg_len)
{
	int head_size = 2;
	*pkg_len = (head_size + len);
	// cache malloc
	//unsigned char* data_buf = (unsigned char*)malloc(*pkg_len);
	unsigned char* data_buf = (unsigned char*)cache_alloc(session_uv::wbuf_allocer, *pkg_len);

	//包头
	data_buf[0] = (unsigned char)*pkg_len & 0x000000ff;
	data_buf[1] = (unsigned char)(*pkg_len & 0x0000ff00) >> 8;

	memcpy(data_buf + head_size, raw_data, len);
	

	return data_buf;
}

void tcp_protocol::release_package(unsigned char* tp_pkg)
{
	//free(tp_pkg);
	cache_free(session_uv::wbuf_allocer, tp_pkg);
}
