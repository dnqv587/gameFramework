#pragma once


/*
serviceģ��---����
*/
class session;
struct cmd_msg;
class service
{
public:
	/*
	service�յ�cmd_msg���д���
	���أ�
		true���ɹ�
		false���ر�socket
	*/
	virtual bool on_session_recv_cmd(session*, cmd_msg* msg);
	//���ߴ���
	virtual void on_session_disconnect(session* s);
	virtual void on_session_disconnect(session* s,int stype);

	virtual bool on_session_recv_raw_cmd(session* s, struct raw_msg* msg);//recv cmd_raw

	bool  using_raw_cmd = false;

};
