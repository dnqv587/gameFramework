#pragma once
#include <vector>
#include <string>
//异步mysql模块

#include <mysql.h>
class mysql_wrapper
{
public:
	/*
	连接数据库
	参数：
	ip：IP地址
	port：端口
	db_name：数据库名
	uname：用户名
	pwd：密码
	open_cb：回调函数
	udata：回调函数参数
	*/
	static void connect(char* ip, int port, char* db_name, char* uname, char* pwd, void(*open_cb)(const char* err, void* Conn, void* udata), void* udata = NULL);
	/*
	关闭数据库
	参数：
	Conn：数据库连接对象
	*/
	static void close(void* Conn);
	/*
	查询数据库
	参数：
	Conn：数据库连接对象
	sql：sql语句
	query_cb：回调函数
	udata：回调函数参数
	*/
	static void query(void* Conn, char* sql, void(*query_cb)(const char* err, MYSQL_RES* result, void* udata), void* udata = NULL);

};
