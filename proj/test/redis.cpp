#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

#include <hiredis.h>
#include "redis.h"
#define NO_QFORKIMPL//必须添加才能使用
#ifdef WIN32
#include <Win32_Interop/win32fixes.h>
#endif



void redis()
{
	//连接redis
	struct timeval timeout = { 1,500000 };//1.5秒
	redisContext* c = redisConnectWithTimeout((char*)"192.168.142.129", 6379, timeout);
	if (c->err)
	{
		std::cerr << "redis connection error:%s" << c->errstr << std::endl;
		return;
	}


	redisReply* replay = (redisReply*)redisCommand(c, "zadd world_rank 1000 xl");
	if (replay)
	{
		std::cout <<replay->type<<"str:" << replay->integer << std::endl;
		freeReplyObject(replay);
	}
	else
	{
		std::cerr << "replay is NULL" << std::endl;
	}

	//关闭连接
	redisFree(c);
}