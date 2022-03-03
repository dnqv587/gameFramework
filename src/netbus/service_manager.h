#pragma once

class session;
class service;
struct cmd_msg;

class service_manager
{
public:
	//��ʼ��
	static void init();
	//ע�����
	/*
	* stype�������
	* s:serviceע�����
	*/
	static bool register_service(int stype, service* s);
	//����cmd_msg��Ϣ
	//����ֵ��true:����ɹ���false���ر�sessoin
	static bool on_recv_cmd_msg(session* s, cmd_msg* msg);
	//�Ͽ����Ӵ�����---�㲥������service
	static void on_session_disconnect(session* s);

private:
	//�洢service
	static service* g_service_set[];
};
