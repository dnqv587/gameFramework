#pragma once


/*
service模块---基类
*/
class session;
struct cmd_msg;
class service
{
public:
	/*
	service收到cmd_msg进行处理
	返回：
		true：成功
		false：关闭socket
	*/
	virtual bool on_session_recv_cmd(session*, cmd_msg* msg);
	//掉线处理
	virtual void on_session_disconnect(session* s);
	virtual void on_session_disconnect(session* s,int stype);

	virtual bool on_session_recv_raw_cmd(session* s, struct raw_msg* msg);//recv cmd_raw

	bool  using_raw_cmd = false;

};
