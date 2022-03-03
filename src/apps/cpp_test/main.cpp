//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <iostream>
//#include <string>
//
//#include "../../netbus/netbus.h"
//#include "../../netbus/proto_man.h"
//#include "proto/pf_cmd_map.h"
//#include "../../utils/logger.h"
//#include "../../utils/time_list.h"
//#include "../../database/mysql_wrapper.h"
//#include "../../database/redis_wrapper.h"
//#include "../../lua/lua_wrapper.h"
//
//
//using namespace std;
//
//static void on_logger_timer(void* udata)
//{
//	log_debug("on_logger_timer");
//}
//
//static void query_cb(const char* err, std::vector<std::vector<std::string>>* result, void* udata)
//{
//	if (err)
//	{
//		std::cout << err << std::endl;
//		return;
//	}
//	for (auto vec : *result)
//	{
//		for (auto str : vec)
//		{
//			std::cout << str<<" ";
//		}
//		std::cout << std::endl;
//	}
//}
//
//static void set_cb(const char* err, std::vector<std::vector<std::string>>* result, void* udata)
//{
//	if (err)
//	{
//		std::cout << err << std::endl;
//		return;
//	}
//
//	std::cout << "set success" << std::endl;
//}
//
//static void query_redis(const char* err, redisReply* result, void* udata)
//{
//	if (err)
//	{
//		std::cout << err << std::endl;
//		return;
//	}
//
//	std::cout << "redis success" << std::endl;
//}
//static void open_cb(const char* err, void* Conn, void* udata)
//{
//	if (err)
//	{
//		printf("%s\n", err);
//		return;
//	}
//
//	printf("connect success\n");
//	//redis_wrapper::close(Conn);
//	redis_wrapper::query(Conn, "select 1", query_redis);
//
//	//mysql_wrapper::query(Conn, "select * from person", query_cb, NULL);
//	//mysql_wrapper::query(Conn, "update person set name=\"shuaige\" where sex = \"man\"", set_cb, NULL);
//	//mysql_wrapper::close(Conn);
//}
//
//static void test_db()
//{
//	//mysql_wrapper::connect("127.0.0.1", 3306, "test", "root", "20171028", open_cb, NULL);
//
//	redis_wrapper::connect("127.0.0.1", 6379, open_cb,NULL);
//	
//}
//
//int main(int argc, char* argv[])
//{
//	//test_db();
//	netbus* test = netbus::instance();
//	proto_man::init(PROTO_BUF);
//	init_pf_cmd_map();
//	logger::init("conf/log", "log", true);
//
//	//schedule_repeat(on_logger_timer, NULL, 0, -1, 3000);
//
//	test->init();
//	test->tcp_listen(8080);
//	test->ws_listen(6080);
//	test->udp_listen(9090);
//
//	lua_wrapper::init();
//	lua_wrapper::do_file("./main.lua");
//
//	test->run();
//
//	lua_wrapper::exit();
//
//	return 0;
//}
