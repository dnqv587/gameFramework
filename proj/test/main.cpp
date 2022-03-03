#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>


#include "libuv.h"
#include "http.h"
#include "redis.h"
#include "win_sock.h"




int main(int argc, char* argv[])
{
	int ret;
#ifdef WIN32
	//配置windows socket版本
	WORD wVersionRequested;
	WSADATA WSAData;
	wVersionRequested = MAKEWORD(2, 2);
	ret = WSAStartup(wVersionRequested, &WSAData);
	if (ret != 0)
	{
		perror("WSAStartup error");
		return -1;
	}
#endif



	//UDP();
	//timer_test();
	//timer_indi();
	//file();
	//std_out_in();
	//http();
	//TCP();
	//websocket();

	//http_server();
	//redis();

	//workQueue();
	sock();


	//结束时清理定义的版本
#ifdef WIN32
	WSACleanup();
#endif

	system("pause");

	return 0;
}