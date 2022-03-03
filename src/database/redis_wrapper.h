#pragma once
#include <vector>
#include <string>
#include <hiredis.h>

//异步redis模块

class redis_wrapper
{
public:
	/*
	连接数据库
	参数：
	ip：IP地址
	port：端口
	open_cb：回调函数
	udata：回调函数参数
	*/
	static void connect(char* ip, int port, void(*open_cb)(const char* err, void* Conn, void* udata), void* udata = NULL);
	/*
	关闭数据库
	参数：
	Conn：数据库连接对象
	*/
#pragma push_macro("close")
#undef close
	static void close(void* Conn);
#pragma pop_macro("close")
	/*
	查询数据库
	参数：
	Conn：数据库连接对象
	cmd：语句
	query_cb：回调函数
	udata：回调函数参数
	*/
	static void query(void* Conn, char* cmd, void(*query_cb)(const char* err, redisReply* result, void* udata), void* udata = NULL);

};

