#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <uv.h>
#include "session.h"

//单例
class netbus
{
public:
	static netbus* instance();

	//初始化
	void init();
	//监听tcp服务器端口
	void tcp_listen(int port);
	//监听udp服务器接口
	void udp_listen(int port);
	//监听websocket服务器端口
	void ws_listen(int port);
	//连接tcp服务器
	void tcp_connect(const char* ip, int port, void (*connect)(int err, session* s, void* udata), void* udata);

	//开启事件循环
	void run();
private:
	uv_tcp_t* m_tcp;
	uv_tcp_t* m_ws;
	uv_udp_t* m_udp;

};