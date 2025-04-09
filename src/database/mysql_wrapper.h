#pragma once
#include <vector>
#include <string>
//�첽mysqlģ��

#include <mysql/mysql.h>
class mysql_wrapper
{
public:
	/*
	�������ݿ�
	������
	ip��IP��ַ
	port���˿�
	db_name�����ݿ���
	uname���û���
	pwd������
	open_cb���ص�����
	udata���ص���������
	*/
	static void connect(char* ip, int port, char* db_name, char* uname, char* pwd, void(*open_cb)(const char* err, void* Conn, void* udata), void* udata = NULL);
	/*
	�ر����ݿ�
	������
	Conn�����ݿ����Ӷ���
	*/
	static void close(void* Conn);
	/*
	��ѯ���ݿ�
	������
	Conn�����ݿ����Ӷ���
	sql��sql���
	query_cb���ص�����
	udata���ص���������
	*/
	static void query(void* Conn, char* sql, void(*query_cb)(const char* err, MYSQL_RES* result, void* udata), void* udata = NULL);

};
