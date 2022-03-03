#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "session.h"
#include "proto_man.h"
#include "service.h"
#include "service_manager.h"

#define MAX_SERVICE 1024

service* service_manager::g_service_set[MAX_SERVICE];

void service_manager::init()
{
	memset(g_service_set, 0x00, sizeof(g_service_set));
}

bool service_manager::register_service(int stype, service* s)
{
	if (stype < 0 || stype >= MAX_SERVICE)
	{
		return false;
	}
	if (g_service_set[stype])//±»×¢²á¹ý
	{
		return false;
	}
	g_service_set[stype] = s;

	return true;
}

bool service_manager::on_recv_cmd_msg(session* s, cmd_msg* msg)
{
	if (g_service_set[msg->stype] == nullptr)
	{
		return false;
	}

	return g_service_set[msg->stype]->on_session_recv_cmd(s, msg);
}

void service_manager::on_session_disconnect(session* s)
{
	for (int i = 0; i < MAX_SERVICE; ++i)
	{
		if (g_service_set[i])
		{
			g_service_set[i]->on_session_disconnect(s);
		}
	}
}
