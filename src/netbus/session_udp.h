#pragma once
#include "session.h"

struct cmd_msg;
class session_udp: public session
{
public:
	//�رսӿ�
	virtual void close() override ;
	//��������
	virtual void send_data(unsigned char* body, int len) override;
	//����cmd_msg
	virtual void send_msg(cmd_msg* msg) override;
	virtual void send_raw_cmd(raw_msg* msg) override;
	//��ȡ��ַ
	virtual const char* get_address(int* client_port) override;


public:
	char c_address[32];
	int c_port;
	const struct sockaddr* addr;
	uv_udp_t* udp_handle;//udp���
private:
	

};


