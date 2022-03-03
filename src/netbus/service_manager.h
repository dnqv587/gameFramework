#pragma once

class session;
class service;
struct cmd_msg;

class service_manager
{
public:
	//初始化
	static void init();
	//注册服务
	/*
	* stype：服务号
	* s:service注册对象
	*/
	static bool register_service(int stype, service* s);
	//处理cmd_msg消息
	//返回值：true:处理成功；false：关闭sessoin
	static bool on_recv_cmd_msg(session* s, cmd_msg* msg);
	//断开连接处理函数---广播给所有service
	static void on_session_disconnect(session* s);

private:
	//存储service
	static service* g_service_set[];
};
