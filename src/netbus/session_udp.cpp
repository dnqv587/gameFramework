#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>
#include <uv.h>

#include "session_udp.h"
#include "proto_man.h"

void session_udp::close()
{

}

static void on_uv_udp_send_end(uv_udp_send_t* req, int status) {
	if (status == 0) {
		// printf("send sucess\n");
	}
	free(req);
}

void session_udp::send_data(unsigned char* body, int len)
{
	uv_buf_t w_buf;
	w_buf = uv_buf_init((char*)body, len);
	//cache
	uv_udp_send_t* req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));

	uv_udp_send(req, udp_handle, &w_buf, 1, addr, on_uv_udp_send_end);
}

void session_udp::send_msg(cmd_msg* msg)
{
	unsigned char* encode_pkg = NULL;
	int encode_len = 0;
	msg->stype = 1;
	msg->ctype = 2;
	encode_pkg = proto_man::encode_msg_to_raw(msg, &encode_len);
	if (encode_pkg)
	{
		send_data(encode_pkg, encode_len);
		proto_man::msg_raw_free(encode_pkg);
	}
}

void session_udp::send_raw_cmd(raw_msg* msg)
{
	send_data(msg->raw_data, msg->raw_len);
}

const char* session_udp::get_address(int* client_port)
{
	*client_port = c_port;
	return c_address;
}
