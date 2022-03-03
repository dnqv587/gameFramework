#pragma once
#include "../utils/cache_alloc.h"
//TCPЭ��ģ��

class tcp_protocol
{
public:
	//��ȡ��ͷ
	/*
	data:����
	data_len�����ݳ���
	pkg_size����ȡ�İ���--����
	out_header_size����ͷ�ĳ���
	*/
	static bool read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size);
	//�������
	static unsigned char* package(const unsigned char* raw_data, int len, int* pkg_len);
	//�ͷ������ڴ�
	static void release_package(unsigned char* tp_pkg);
};