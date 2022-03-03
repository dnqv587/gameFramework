
#include "netbus.h"
#include "session_uv.h"
#include "ws_protocol.h"
#include "tcp_protocol.h"
#include "proto_man.h"
#include "service_manager.h"
#include "session_udp.h"
#include "../utils/logger.h"

using namespace std;

//单例对象
static netbus* g_instance = nullptr;
netbus* netbus::instance()
{
	if (nullptr == g_instance)
	{
		g_instance = new netbus;
		return g_instance;
	}
	return g_instance;
}


extern "C"
{

	//执行command
	static void on_recv_client_cmd(session* s, unsigned char* body, int len)
	{
		//std::cout << "client command !!" << std::endl;
		log_debug("client command !!");
	
		cmd_msg* msg = NULL;
		if (proto_man::decode_cmd_msg(body, len, &msg))//解析protobuf报文
		{

			if (!service_manager::on_recv_cmd_msg((session*)s, msg))
			{
				s->close();
			}
			proto_man::cmd_msg_free(msg);
		}


	}

	//解析tcp数据
	static void on_recv_tcp_data(session_uv* s)
	{
		//数据包大于4096则存long_pkg里
		unsigned char* pkg_data = (unsigned char*)(s->long_pkg != nullptr ? s->long_pkg : s->recv_buf);
		//循环处理接收到的数据
		while (s->recved > 0)
		{
			int pkg_size = 0;
			int head_size = 0;

			if (!tcp_protocol::read_header(pkg_data, s->recved, &pkg_size, &head_size))
			{
				break;
			}
			if (s->recved < pkg_size)//收到的数据小于数据包---没收完
			{
				break;
			}

			unsigned char* raw_data = pkg_data + head_size;

			//收到一个完整的command
			on_recv_client_cmd((session*)s, raw_data, pkg_size - head_size);

			if (s->recved > pkg_size)
			{
				memmove(pkg_data, pkg_data + pkg_size, s->recved - pkg_size);//去除掉解析完的数据
			}
			s->recved -= pkg_size;//剩余长度

			//释放内存
			if (s->recved == 0 && s->long_pkg != nullptr)
			{
				free(s->long_pkg);
				s->long_pkg = nullptr;
				s->long_pkg_size = 0;
			}

		}
	}




	//解析websocket数据
	static void on_recv_ws_data(session_uv* s)
	{
		//数据包大于4096则存long_pkg里
		unsigned char* pkg_data = (unsigned char*)(s->long_pkg != nullptr ? s->long_pkg : s->recv_buf);
		//循环处理接收到的数据
		while (s->recved > 0)
		{
			int pkg_size = 0;
			int head_size = 0;
			
			if (pkg_data[0] == 0x88)//关闭协议
			{
				s->close();
				break;
			}
			//pkg_size-head_size=body_size
			if (!ws_protocol::read_ws_header(pkg_data, s->recved, &pkg_size, &head_size))
			{
				break;
			}
			if (s->recved < pkg_size)//收到的数据小于数据包---没收完
			{
				break;
			}


			unsigned char* raw_data = pkg_data + head_size;
			unsigned char* mask = raw_data - 4;

			ws_protocol::parser_ws_recv_data(raw_data, mask, pkg_size - head_size);//解析数据

			//收到一个完整的命令包
			on_recv_client_cmd((session*)s, raw_data, pkg_size - head_size);

			if (s->recved > pkg_size)
			{
				memmove(pkg_data, pkg_data + pkg_size, s->recved - pkg_size);//去除掉解析完的数据
			}
			s->recved -= pkg_size;//剩余长度

			//释放内存
			if (s->recved == 0 && s->long_pkg != nullptr)
			{
				free(s->long_pkg);
				s->long_pkg = nullptr;
				s->long_pkg_size = 0;
			}

		}
	}

	/*
说明：申请读入数据的内存
参数：
	1：发生读事件的句柄
	2：建议分配多大的内存，来保存数据
	3：准备好的内存，通过uv_buf_t，告诉event_loop申请的内存地址
*/
	static void tcp_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		session_uv* s = (session_uv*)handle->data;

		if (s->recved < RECV_LEN)//数据小于4096
		{
			*buf = uv_buf_init(s->recv_buf + s->recved, RECV_LEN - s->recved);
		}//数据大于4096
		else
		{
			if (s->long_pkg == nullptr)
			{
				if (s->socket_type == WS_SOCKET && s->is_ws_shake)//websocket握完手后接收的大包
				{
					int pkg_size;
					int head_size;
					ws_protocol::read_ws_header((unsigned char*)s->recv_buf, s->recved, &pkg_size, &head_size);//获取ws的数据大小
					s->long_pkg_size = pkg_size;
					s->long_pkg = (char*)malloc(s->long_pkg_size);
					memcpy(s->long_pkg, s->recv_buf, s->recved);
				}
				else//tcp的大包
				{
					int pkg_size;
					int head_size;
					tcp_protocol::read_header((unsigned char*)s->recv_buf, s->recved, &pkg_size, &head_size);
					s->long_pkg_size = pkg_size;
					s->long_pkg = (char*)malloc(s->long_pkg_size);
					memcpy(s->long_pkg, s->recv_buf, s->recved);
				}
			}

			*buf = uv_buf_init(s->long_pkg + s->recved, s->long_pkg_size - s->recved);
		}
	}

	//udp数据结构
	struct udp_recv_buf
	{
		char* recv_buf;
		size_t max_recv_len;
	};

	static void udp_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		suggested_size = suggested_size < 8096 ? 8096 : suggested_size;

		struct udp_recv_buf* udp_buf = (udp_recv_buf*) handle->data;
		if (udp_buf->max_recv_len < suggested_size)
		{
			if (udp_buf->recv_buf)
			{
				free(udp_buf->recv_buf);
				udp_buf->recv_buf = nullptr;
			}
			udp_buf->recv_buf = (char*)malloc(suggested_size);
			udp_buf->max_recv_len = suggested_size;

		}
		buf->base = udp_buf->recv_buf;
		buf->len = suggested_size;
	}


	static void close_cb(uv_handle_t* handle)
	{
		//std::cout << "close client" << std::endl;
		log_debug("close client");
		session_uv* s = (session_uv*)handle->data;

		session_uv::destroy(s);//销毁session_uv对象
		
	}

	static void shutdown_cb(uv_shutdown_t* req, int status)
	{

		uv_close((uv_handle_t*)req->handle, close_cb);//关闭套接字
	}

	//udp读事件回调
	static void udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
	{
		session_udp udp_s;
		udp_s.udp_handle = handle;
		udp_s.addr = addr;
		uv_ip4_name((sockaddr_in*)addr, udp_s.c_address, sizeof(udp_s.c_address));//获得ip地址
		udp_s.c_port= ntohs(((sockaddr_in*)addr)->sin_port);

		on_recv_client_cmd((session*)&udp_s, (unsigned char*)buf->base, nread);
	}

	/*
说明：tcp读事件回调
参数：
	1：读事件句柄
	2：读到了多少字节的数据
	3：数据所在的buf
*/
	static void tcp_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
	{
		session_uv* s = (session_uv*)stream->data;
		//连接断开
		if (nread < 0)
		{
			//std::cout << "client shutdown " << std::endl;
			log_debug("client shutdown ");
			s->close();
			//uv_shutdown_t* req = &s->m_shutdown;
			//memset(req, 0x00, sizeof(uv_shutdown_t));
			//uv_shutdown(req, stream, shutdown_cb);//断开连接
			return;
		}

		buf->base[nread] = 0;

		//std::cout << "recv" << nread << ":" << buf->base << std::endl;

		s->recved += nread;//收到数据的大小
		if (s->socket_type == WS_SOCKET)  //websocket
		{
			if (s->is_ws_shake == 0)//没有握手
			{
				if (ws_protocol::ws_shake_hand((session*)s, s->recv_buf, s->recved))//握手
				{
					s->is_ws_shake = 1;
					s->recved = 0;
				}
			}
			else//websocket收发数据
			{
				on_recv_ws_data(s);
			}
		}
		else  //tcp_socket
		{
			on_recv_tcp_data(s);
		}
		
	}

	static void connection_cb(uv_stream_t* server, int status)
	{


		//接入客户端
		session_uv* s = (session_uv*)session_uv::create();

		uv_tcp_t* client = &s->tcp_handler;
		memset(client, 0x00, sizeof(uv_tcp_t));
		uv_tcp_init(uv_default_loop(), client);

		client->data = (void*)s;//传入session_uv对象

		uv_accept(server, (uv_stream_t*)client);

		//存储客户端信息
		sockaddr_in addr;
		int len = sizeof(addr);
		uv_tcp_getpeername(client, (sockaddr*)&addr, &len);
		uv_ip4_name(&addr, s->c_address, 32);
		s->c_port = ntohs(addr.sin_port);
		s->socket_type = (SockType)((int)server->data);
		//std::cout << "new client coming " << s->c_address << ":" << s->c_port << std::endl;
		log_warning("new client coming ", s->c_address, ":", s->c_port);

		//告诉event_loop,让他管理某个事件

		/*
		说明：管理读事件
		参数：
			1：对象
			2：申请读入数据的内存
		*/
		uv_read_start((uv_stream_t*)client, tcp_alloc_cb, tcp_read_cb);
	}


	struct on_connect_cb 
	{
		void (*connect)(int err, session* s, void* udata);
		void* udata;
	};

	void after_connect(uv_connect_t* req, int status) {
		session_uv* s = (session_uv*)req->handle->data;

		struct on_connect_cb* cb = (struct on_connect_cb*)req->data;
		if (status) {
			if (cb->connect) {
				cb->connect(1, NULL, cb->udata);
			}
			s->close();
			free(cb);
			free(req);
			return;
		}
		if (cb->connect) {
			cb->connect(0, s, cb->udata);
		}

		uv_read_start((uv_stream_t*)req->handle, tcp_alloc_cb, tcp_read_cb);
		free(cb);
		free(req);

	}


}

void netbus::tcp_listen(int port)
{
	m_tcp = new uv_tcp_t;
	memset(m_tcp, 0x00, sizeof(uv_tcp_t));

	// Tcp 监听服务;
	uv_tcp_init(uv_default_loop(), m_tcp);
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr); // ip地址, 端口
	int ret = uv_tcp_bind(m_tcp, (const struct sockaddr*)&addr, 0);
	if (ret != 0) {
		//printf("bind error \n");
		log_error("tcp bind error");
		delete m_tcp;
		return;
	}

	// 让event loop来做监听管理，当我们的listen句柄上有人连接的时候;
	// event loop就会调用用户指定的这个处理函数connection_cb;
	/*参数：
		1:对象
		2：监听的数目，SOMAXCONN：最大数目
	 */
	uv_listen((uv_stream_t*)m_tcp, SOMAXCONN, connection_cb);
	m_tcp->data = (void*)TCP_SOCKET;
}

void netbus::udp_listen(int port)
{
	m_udp = new uv_udp_t;
	memset(m_udp, 0x00, sizeof(uv_udp_t));

	uv_udp_init(uv_default_loop(), m_udp);
	udp_recv_buf* udp_buf = (udp_recv_buf*)malloc(sizeof(udp_recv_buf));
	memset(udp_buf, 0x00, sizeof(udp_recv_buf));
	m_udp->data = (udp_recv_buf*)udp_buf;
	//绑定端口
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr);
	int ret = uv_udp_bind(m_udp, (const sockaddr*)&addr, 0);
	if (ret != 0) {
		//printf("udp bind error \n");
		log_error("udp bind error")
		delete m_udp;
		return;
	}


	uv_udp_recv_start(m_udp, udp_alloc_cb, udp_recv_cb);
}

void netbus::ws_listen(int port)
{
	m_ws = new uv_tcp_t;
	memset(m_ws, 0x00, sizeof(uv_tcp_t));

	//ws监听服务;
	uv_tcp_init(uv_default_loop(), m_ws);
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", port, &addr); // ip地址, 端口
	int ret = uv_tcp_bind(m_ws, (const struct sockaddr*)&addr, 0);
	if (ret != 0) {
		//printf("bind error \n");
		log_error("websocket bind error");
		delete m_ws;
		return;
	}

	// 让event loop来做监听管理，当我们的listen句柄上有人连接的时候;
	// event loop就会调用用户指定的这个处理函数connection_cb;
	/*参数：
		1:对象
		2：监听的数目，SOMAXCONN：最大数目
	 */
	uv_listen((uv_stream_t*)m_ws, SOMAXCONN, connection_cb);
	m_ws->data = (void*)WS_SOCKET;
}

void netbus::tcp_connect(const char* ip, int port, void (*connect)(int err, session* s, void* udata), void* udata)
{
	struct sockaddr_in bind_addr;
	int iret = uv_ip4_addr(ip, port, &bind_addr);
	if (iret) {
		return;
	}

	session_uv* s = session_uv::create();
	uv_tcp_t* client = &s->tcp_handler;
	memset(client, 0, sizeof(uv_tcp_t));
	uv_tcp_init(uv_default_loop(), client);
	client->data = (void*)s;
	s->as_client = 1;
	s->socket_type = TCP_SOCKET;
	strcpy(s->c_address, ip);
	s->c_port = port;

	uv_connect_t* connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));

	struct on_connect_cb* cb = (struct on_connect_cb*)malloc(sizeof(struct on_connect_cb));
	cb->connect = connect;
	cb->udata = udata;
	connect_req->data = cb;

	iret = uv_tcp_connect(connect_req, client, (struct sockaddr*)&bind_addr, after_connect);
	if (iret) {
		// log_error("uv_tcp_connect error!!!");
		return;

	}
}

void netbus::run()
{
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}


void netbus::init()
{
	service_manager::init();
	session_uv::init_session_allocer();
}