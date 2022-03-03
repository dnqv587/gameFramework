#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <uv.h>
#include "session.h"

//����
class netbus
{
public:
	static netbus* instance();

	//��ʼ��
	void init();
	//����tcp�������˿�
	void tcp_listen(int port);
	//����udp�������ӿ�
	void udp_listen(int port);
	//����websocket�������˿�
	void ws_listen(int port);
	//����tcp������
	void tcp_connect(const char* ip, int port, void (*connect)(int err, session* s, void* udata), void* udata);

	//�����¼�ѭ��
	void run();
private:
	uv_tcp_t* m_tcp;
	uv_tcp_t* m_ws;
	uv_udp_t* m_udp;

};