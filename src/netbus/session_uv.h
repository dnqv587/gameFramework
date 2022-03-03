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
	//����
	static session_uv* create();
	//����
	static void destroy(session_uv* s);

	//�رսӿ�
	virtual void close() override;
	//��������
	virtual void send_data(unsigned char* body, int len) override;
	//����cmd_msg
	virtual void send_msg(cmd_msg* msg) override;
	//��ȡ��ַ
	virtual const char* get_address(int* client_port) override;

	//��ʼ���ڴ�
	static void init_session_allocer();

private:
	void init();
	void exit();

public:
	uv_tcp_t tcp_handler;
	char c_address[32];//�ͻ���ip��ַ
	int c_port;//�ͻ���port
	char recv_buf[RECV_LEN];//���յ�������
	int recved;//�յ����ݵĳ���
	char* long_pkg;//������--���ȴ���RECV_LEN
	int long_pkg_size;//�����ݳ���
	SockType socket_type;//�������
	uv_shutdown_t m_shutdown;//�رվ��
	bool is_shutdown;//�Ƿ��Ѿ��رվ��
	int is_ws_shake;//�Ƿ��Ѿ�����

	//uv_write_t w_req;//д���

	static cache_allocer* session_allocer;//���session����
	static cache_allocer* wr_allocer;
	static cache_allocer* wbuf_allocer;
	
};



