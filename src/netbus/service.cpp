#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service.h"
#include "session.h"
#include "proto_man.h"

bool service::on_session_recv_cmd(session*, cmd_msg* msg)
{
	return false;
}

void service::on_session_disconnect(session* s)
{

}

void service::on_session_disconnect(session* s, int stype)
{

}

bool service::on_session_recv_raw_cmd(session* s, struct raw_msg* msg)
{
	return false;
}
