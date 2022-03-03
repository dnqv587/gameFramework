#pragma once
#include "session.h"

struct cmd_msg;
class session_udp: public session
{
public:
	//关闭接口
	virtual void close() override ;
	//发送数据
	virtual void send_data(unsigned char* body, int len) override;
	//发送cmd_msg
	virtual void send_msg(cmd_msg* msg) override;
	virtual void send_raw_cmd(raw_msg* msg) override;
	//获取地址
	virtual const char* get_address(int* client_port) override;


public:
	char c_address[32];
	int c_port;
	const struct sockaddr* addr;
	uv_udp_t* udp_handle;//udp句柄
private:
	

};


