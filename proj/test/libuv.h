#pragma once
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <Ws2tcpip.h>

#include "uv.h"

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#pragma comment (lib,"Advapi32.lib")
#pragma comment (lib,"Iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "userenv.lib")
#pragma comment(lib,"Ws2_32.lib")
#endif


//获取当前系统从开机到现在运行时间
#ifdef WIN32
#include <windows.h>
inline static unsigned int get_cur_ms()//单位毫秒
{
	return GetTickCount64();
}
#else
#include <sys/time.h>
#include <time.h>
#include <limits.h>

static unsigned int get_cur_ms()
{
	struct timeval tv;
	//struct timezone tz;
	gettimeofday(&tv, NULL);

	return ((tv.tv_usec / 1000) + tv.tv_sec * 1000);
}

#endif

void websocket();

void  UDP();
void TCP();
void timer_test();
void timer_indi();
void file();
void std_out_in();
void http_server();

void workQueue();