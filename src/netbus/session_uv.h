#pragma once
#include <uv.h>
#include "session.h"
#include "../utils/cache_alloc.h"

#define RECV_LEN  4096

enum SockType
{
	TCP_SOCKET,
	WS_SOCKET
};

class session_uv :public session 
{
public:
	//创建
	static session_uv* create();
	//销毁
	static void destroy(session_uv* s);

	//关闭接口
	virtual void close() override;
	//发送数据
	virtual void send_data(unsigned char* body, int len) override;
	//发送cmd_msg
	virtual void send_msg(cmd_msg* msg) override;
	//获取地址
	virtual const char* get_address(int* client_port) override;

	//初始化内存
	static void init_session_allocer();

private:
	void init();
	void exit();

public:
	uv_tcp_t tcp_handler;
	char c_address[32];//客户端ip地址
	int c_port;//客户端port
	char recv_buf[RECV_LEN];//接收到的数据
	int recved;//收到数据的长度
	char* long_pkg;//长数据--长度大于RECV_LEN
	int long_pkg_size;//长数据长度
	SockType socket_type;//句柄类型
	uv_shutdown_t m_shutdown;//关闭句柄
	bool is_shutdown;//是否已经关闭句柄
	int is_ws_shake;//是否已经握手

	//uv_write_t w_req;//写句柄

	static cache_allocer* session_allocer;//存放session对象
	static cache_allocer* wr_allocer;
	static cache_allocer* wbuf_allocer;
	
};



