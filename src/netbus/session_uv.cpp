#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>

#include "session_uv.h"
#include "ws_protocol.h"
#include "../utils/cache_alloc.h"
#include "tcp_protocol.h"
#include "proto_man.h"
#include "service_manager.h"

using namespace std;

#define SESSION_CACHE_CAPACITY 6000
#define WQ_CACHE_CAPACITY 4096
#define WBUF_CACHE_CAPACITY 1024
#define CMD_CACHE_SIZE 1024

cache_allocer* session_uv::session_allocer = nullptr;
cache_allocer* session_uv::wr_allocer = nullptr;
cache_allocer* session_uv::wbuf_allocer = nullptr;


extern "C"
{
	//写事件回调
	static void tcp_write_cb(uv_write_t* req, int status)
	{
		if (status == 0)
			std::cout << "send success" << std::endl;

		cache_free(session_uv::wr_allocer, req);//释放内存
	}

	static void close_cb(uv_handle_t* handle)
	{
		std::cout << "close client" << std::endl;
		session_uv* s = (session_uv*)handle->data;

		session_uv::destroy(s);//销毁session_uv对象

	}

	static void shutdown_cb(uv_shutdown_t* req, int status)
	{

		uv_close((uv_handle_t*)req->handle, close_cb);//关闭套接字
	}
}



void session_uv::init_session_allocer()
{
	if (session_allocer == NULL) {
		session_allocer = create_cache_allocer(SESSION_CACHE_CAPACITY, sizeof(session_uv));
	}
	if (wr_allocer == NULL) {
		wr_allocer = create_cache_allocer(WQ_CACHE_CAPACITY, sizeof(uv_write_t));
	}
	if (wbuf_allocer == NULL) {
		wbuf_allocer = create_cache_allocer(WBUF_CACHE_CAPACITY, sizeof(CMD_CACHE_SIZE));
	}
}

void session_uv::close()
{
	if (this->is_shutdown)
		return;

	//广播service,sessoin断线
	service_manager::on_session_disconnect((session*)this);

	is_shutdown = true;
	uv_shutdown_t* req = &m_shutdown;
	memset(req, 0x00, sizeof(uv_shutdown_t));
	uv_shutdown(req, (uv_stream_t*)&tcp_handler, shutdown_cb);//断开连接
}

void session_uv::send_data(unsigned char* body, int len)
{
	//写数据
	uv_write_t* req = (uv_write_t*)cache_alloc(wr_allocer, sizeof(uv_write_t));
	uv_buf_t w_buf;

	if (socket_type == WS_SOCKET)//websocket
	{
		if (is_ws_shake)
		{
			int ws_pkg_len;
			unsigned char* ws_pkg = ws_protocol::package_ws_send_data(body, len, &ws_pkg_len);
			w_buf = uv_buf_init((char*)ws_pkg, ws_pkg_len);
			uv_write(req, (uv_stream_t*)&tcp_handler, &w_buf, 1, tcp_write_cb);
			ws_protocol::free_ws_send_pkg(ws_pkg);
		}
		else//发送握手
		{
			w_buf = uv_buf_init((char*)body, len);
			uv_write(req, (uv_stream_t*)&tcp_handler, &w_buf, 1, tcp_write_cb);
		}
		
	}
	else//tcp
	{
		int tcp_pkg_len;
		unsigned char* tcp_pkg = tcp_protocol::package(body, len, &tcp_pkg_len);
		w_buf = uv_buf_init((char*)tcp_pkg, tcp_pkg_len);
		uv_write(req, (uv_stream_t*)&tcp_handler, &w_buf, 1, tcp_write_cb);
		tcp_protocol::release_package(tcp_pkg);
	}
	
}

void session_uv::send_msg(cmd_msg* msg)
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

const char* session_uv::get_address(int* client_port)
{
	*client_port = c_port;
	return c_address;
}

session_uv* session_uv::create()
{
	//session_uv* uv_s = new session_uv();
	//内存池动态管理
	session_uv* uv_s = (session_uv*)cache_alloc(session_allocer, sizeof(session_uv));
	uv_s->session_uv::session_uv();
	uv_s->init();
	return uv_s;
}

void session_uv::destroy(session_uv* s)
{
	s->exit();
	s->session_uv::~session_uv();
	//销毁内存池对象
	cache_free(session_allocer, s);
	//delete s;
}

void session_uv::init()
{
	memset(c_address, 0x00, sizeof(c_address));
	memset(recv_buf, 0x00, sizeof(char) * RECV_LEN);
	c_port = 0;
	recved = 0;
	is_shutdown = false;
	is_ws_shake = 0;
	long_pkg = nullptr;
	long_pkg_size = 0;

}

void session_uv::exit()
{

}


