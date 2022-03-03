#pragma once
#include <vector>
#include <string>
#include <hiredis.h>

//�첽redisģ��

class redis_wrapper
{
public:
	/*
	�������ݿ�
	������
	ip��IP��ַ
	port���˿�
	open_cb���ص�����
	udata���ص���������
	*/
	static void connect(char* ip, int port, void(*open_cb)(const char* err, void* Conn, void* udata), void* udata = NULL);
	/*
	�ر����ݿ�
	������
	Conn�����ݿ����Ӷ���
	*/
#pragma push_macro("close")
#undef close
	static void close(void* Conn);
#pragma pop_macro("close")
	/*
	��ѯ���ݿ�
	������
	Conn�����ݿ����Ӷ���
	cmd�����
	query_cb���ص�����
	udata���ص���������
	*/
	static void query(void* Conn, char* cmd, void(*query_cb)(const char* err, redisReply* result, void* udata), void* udata = NULL);

};

