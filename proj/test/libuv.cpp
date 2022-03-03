#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include<fcntl.h>
#include <direct.h>


#include "libuv.h"


static uv_loop_t* event_loop = NULL;//�¼�ѭ������
static uv_udp_t server;//UDP�ľ��
static uv_tcp_t tcp_server;//tcp���
static uv_timer_t timer;//timer���


//-------------------------------------------TCP-----------------------------------------------
/*
uv_stream_t���ݽṹ��
struct uv_stream_s {
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
};

uv_tcp_t���ݽṹ��
struct uv_tcp_s {
  UV_HANDLE_FIELDS
  UV_STREAM_FIELDS
  UV_TCP_PRIVATE_FIELDS
};

uv_handle_t���ݽṹ
struct uv_handle_s {
  UV_HANDLE_FIELDS
};

ǿת�ݶ�---�̳й�ϵ
uv_handle_t > uv_stream_t > uv_tcp_t
*/

/*
˵��������������ݵ��ڴ�
������
	1���������¼��ľ��
	2�������������ڴ棬����������
	3��׼���õ��ڴ棬ͨ��uv_buf_t������event_loop������ڴ��ַ
*/
static void tcp_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	if (handle->data != NULL)//�ͷ��ϴζ���������
	{
		free(handle->data);
		handle->data = nullptr;
	}
	handle->data = malloc(suggested_size + 1);//suggested_size����Ĵ�С��+1�š�\0��
	buf->base = (char*)handle->data;
	buf->len = suggested_size;
}

static void close_cb(uv_handle_t* handle)
{
	if (handle->data)
	{
		free(handle->data);//�ͷ����һ�ζ���������
		handle->data = nullptr;
	}

	free(handle);

	std::cout << "close client" << std::endl;
}

static void shutdown_cb(uv_shutdown_t* req, int status)
{
	
	uv_close((uv_handle_t*)req->handle, close_cb);//�ر��׽���
	free(req);
}

static void tcp_write_cb(uv_write_t* req, int status)
{
	if(status==0)
		std::cout << "send success" << std::endl;

	free(req);
}

/*
˵�������¼��ص�
������
	1�����¼����
	2�������˶����ֽڵ�����
	3���������ڵ�buf
*/
static void tcp_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	//���ӶϿ�
	if (nread < 0)
	{
		uv_shutdown_t* req = (uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
		memset(req, 0x00, sizeof(uv_shutdown_t));
		uv_shutdown(req, stream, shutdown_cb);//�Ͽ�����
		return;
	}

	buf->base[nread] = 0;

	std::cout << "recv" << nread << ":" << buf->base << std::endl;

	//д����
	uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
	uv_buf_t send_buf= uv_buf_init((char*)"send data", sizeof("send data"));
	uv_write(req, stream, &send_buf, 1, tcp_write_cb);
}

static void connection_cb(uv_stream_t* server, int status)
{
	std::cout << "new client coming" << std::endl;

	//����ͻ���
	uv_tcp_t* client =(uv_tcp_t*) malloc(sizeof(uv_tcp_t));
	memset(client, 0x00, sizeof(uv_tcp_t));
	uv_tcp_init(event_loop, client);
	uv_accept(server, (uv_stream_t*)client);

	//����event_loop,��������ĳ���¼�
	
	/*
	˵����������¼�
	������
		1������
		2������������ݵ��ڴ�
	*/
	uv_read_start((uv_stream_t*)client, tcp_alloc_cb, tcp_read_cb);
}

void TCP()
{
	event_loop = uv_default_loop();//����Ĭ�ϵ��¼�loop
	
	memset(&tcp_server, 0x00, sizeof(uv_tcp_t));

	uv_tcp_init(event_loop, &tcp_server);//��ʼ��udp

	//�󶨶˿�
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", 8080, &addr);
	int ret = uv_tcp_bind(&tcp_server, (sockaddr*)&addr, 0);
	if (ret != 0)
	{
		goto FAILED;
	}

	/*
	������
		1:����
		2����������Ŀ��SOMAXCONN�������Ŀ
	*/
	uv_listen((uv_stream_t*)&tcp_server, SOMAXCONN, connection_cb);

	uv_run(event_loop, UV_RUN_DEFAULT);//�����¼�ѭ���ȴ��¼��ķ���

FAILED:
	std::cout << "end" << std::endl;

}


//-----------------------------------------UDP--------------------------------------------------
//����ص�����
static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	if (handle->data != NULL)
	{
		free(handle->data);
		handle->data = NULL;
	}
	handle->data = malloc(suggested_size + 1);//suggested_size����Ĵ�С��+1�š�\0��
	buf->base = (char*)handle->data;
	buf->len = suggested_size;
}


//send����ص�����
static void udp_send_cb(uv_udp_send_t* req, int status)
{
	if (status == 0)//���ͳɹ�
	{
		std::cout << "send success" << std::endl;
	}
	free(req);

}

//recv����ص�����
static void udp_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
	char ip[128];
	uv_ip4_name((sockaddr_in*)addr, ip, sizeof(ip));
	int port = ntohs(((sockaddr_in*)addr)->sin_port);
	printf("ip:%s:%d nread = % d\n", ip, port, nread);
	char* str_buf = (char*)handle->data;
	str_buf[nread] = '\0';
	std::cout << "recv:" << str_buf << std::endl;


	uv_buf_t w_buf;

	w_buf = uv_buf_init((char*)"send data", sizeof("send data"));
	//д����
	uv_udp_send_t* req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
	uv_udp_send(req, handle, &w_buf, 1, addr, udp_send_cb);

}



void  UDP()
{
	event_loop = uv_default_loop();//����Ĭ�ϵ��¼�loop
	memset(&server, 0x00, sizeof(uv_udp_t));

	uv_udp_init(event_loop, &server);//��ʼ��udp

	//�󶨶˿�
	struct sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", 8080, &addr);
	uv_udp_bind(&server, (sockaddr*)&addr, 0);

	//����ʱ��ѭ����Ҫ������¼�
	uv_udp_recv_start(&server, alloc_cb, udp_recv_cb);

	

	uv_run(event_loop, UV_RUN_DEFAULT);//�����¼�ѭ���ȴ��¼��ķ���

}

//-----------------Timer-----------
static void timerTest_cb(uv_timer_t* handle)
{
	std::cout << "timer called" << std::endl;
	//uv_timer_stop(handle); //ֹͣtimer���
}

void timer_test()
{
	event_loop = uv_default_loop();//����Ĭ�ϵ��¼�loop

	uv_timer_init(event_loop, &timer);//��ʼ��timer���

	uv_timer_start(&timer, timerTest_cb, 5000, 0);//timeout:��һ�μ������ʱ�䣻repeat��֮����������ʱ��--0Ϊֻ����һ��

	uv_run(event_loop, UV_RUN_DEFAULT);

}


//�Զ���timer

struct Timer
{
	uv_timer_t uv_timer;//libuv������
	void (*timer_cb)(void* udata);
	void* udata;
	int repeat_count;//-1һֱѭ��
};

//����Timer�ṹ��
Timer* alloc_timer(void (*timer_cb)(void* udata), void* udata, int repeat_count)
{
	Timer* t = (Timer*)malloc(sizeof(Timer));
	memset(t, 0x00, sizeof(Timer));

	t->timer_cb = timer_cb;
	t->udata = udata;
	t->repeat_count = repeat_count;

	uv_timer_init(uv_default_loop(), &t->uv_timer); //��ʼ�����

	return t;
}

void free_timer(Timer* t)
{
	free(t);
}

void timer_cb(uv_timer_t* handle)
{
	Timer* t = (Timer*)handle->data;
	if (t->repeat_count > 0)
	{
		--t->repeat_count;
		t->timer_cb(t->udata);//�ݹ�
	}
	if (t->repeat_count == 0)
	{
		uv_timer_stop(&t->uv_timer); //ֹͣtimer���
		free_timer(t);//�ͷ�
	}

	t->timer_cb(t->udata);//repeat_countΪ-1��һֱִ��
}

/*
������
void (*timer_cb)(void* udata)���ص���������timer����ʱ�ص�
void* udata���û������Զ�������ݽṹ���ص�����ִ�е�ʱ���udata�������udata
float after_sec�������뿪ʼִ��
int repeat_count��ִ�ж��ٴΣ�-1����һֱִ��
����:timer�ľ��
*/
//����timer
Timer* schedule(void (*timer_cb)(void* udata), void* udata,int after_sec,int repeat_count)
{
	Timer* t = alloc_timer(timer_cb, udata, repeat_count);

	//libuv����timer
	t->uv_timer.data = t;//�洢Timer����
	uv_timer_start(&t->uv_timer, (uv_timer_cb)timer_cb, after_sec , after_sec );

	return t;
}


//�ص�����ִֻ��һ��
Timer* schedule_once(void (*timer_cb)(void* udata), void* udata, int after_sec)
{
	return schedule(timer_cb, udata, after_sec, 1);
}

//ȡ��timer
void  cancel_timer(Timer* t)
{
	if (t->repeat_count == 0)//ȫ���������
	{
		return;
	}
	uv_timer_stop(&t->uv_timer);
	free(t);
}

void callback_timer(void* data)
{
	char* str = (char*)data;
	std::cout << str << std::endl;
}

void timer_indi()
{
	event_loop = uv_default_loop();//����Ĭ�ϵ��¼�loop

	Timer* t = schedule(callback_timer, (char*)"Hello", 1000, 5);


	uv_run(event_loop, UV_RUN_DEFAULT);
}

//-------------------------------------------�첽�ļ���д-----------------------------------------------


static uv_fs_t req;

//uv_file :�ļ�������󣬴��ļ����handle
static uv_file fs_handle;


static char buf[1024];

//��ȡ�ļ���Ļص�����
void read_fs_cb(uv_fs_t* req)
{
	
	std::cout << "read :" << req->result << std::endl;

	buf[req->result] = 0;//�ַ����ļ���β
	std::cout << buf << std::endl;

	uv_fs_req_cleanup(req);

	uv_fs_close(event_loop, req, fs_handle, NULL);
	uv_fs_req_cleanup(req);
}

//���ļ���Ļص�����
void open_fs_cb(uv_fs_t* req)
{
	//result:ÿ������Ľ���������ֵ�����أ����ļ�---result���ش��ļ��������uv_file
		//���ļ���result�������ݵĳ���
		//д�ļ���resultΪд�����ݵĳ���
	fs_handle = req->result;

	uv_fs_req_cleanup(req);//�ͷ��������req��ռ����Դ

	std::cout << "open success" << std::endl;

	//�ر��ļ����
	//uv_fs_close(event_loop, req, fs_handle, NULL);
	//uv_fs_req_cleanup(req);

	//��ȡ�ļ�
	
	uv_buf_t mem_buf = uv_buf_init(buf, 1024);
	uv_fs_read(event_loop, req, fs_handle, &mem_buf, 1, 0, read_fs_cb);

}

void file()
{
	event_loop = uv_default_loop();

	//���ļ�
	/*
	loop���¼�ѭ��
	uv_fs_t:req�������
	path���ļ�·��
	flags����־--һ��Ϊ0
	mode���ɶ�����д...����Ҫ#include<fcntl.h>
	cb:�ص�����
	*/
	std::string path = getcwd(NULL, 0);
	path += "/test.txt";

	uv_fs_open(event_loop, &req, path.c_str(), 0, O_RDONLY, open_fs_cb);


	uv_run(event_loop, UV_RUN_DEFAULT);
}

//---------------------------------------�첽��׼�������---------------------------------

static uv_fs_t w_req;
void std_out_in()
{
	event_loop = uv_default_loop();

	uv_buf_t w_buf = uv_buf_init((char*)"Good by\n", 8);
	uv_fs_write(event_loop, &w_req, (uv_file)1, &w_buf, 1, 0, NULL);//stdin---0;stdout---1
	uv_fs_req_cleanup(&w_req);//����Ҫ�ص������������


	uv_run(event_loop, UV_RUN_DEFAULT);
}