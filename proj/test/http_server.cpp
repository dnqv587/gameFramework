#include <iostream>
#include <string>
#include <string.h>
#include <vector>

#include "libuv.h"
#include "../../3rd/http_parser/http_parser.h"

static uv_loop_t* event_loop = NULL;//�¼�ѭ������

static uv_tcp_t tcp_server;//tcp���


/*
urlע�����ģ��
*/
typedef void (*web_get_handle)(uv_stream_t* stream, char* url);
typedef void (*web_post_handle)(uv_stream_t* stream, char* url);

struct url_node
{
	char* url;//url��ַ
	web_get_handle get;//url��Ӧ�Ĵ�����
	web_post_handle post;
};

static struct url_node* alloc_url_node(char* url, int len, web_get_handle get, web_post_handle post)//
{
	struct url_node* node = (url_node*)malloc(sizeof(url_node));
	memset(node, 0x00, sizeof(url_node));
	memcpy(node->url, url, len * sizeof(char));
	node->url[len] = 0;
	node->get = get;
	node->post = post;

	return node;
}

static std::vector<url_node> urlNode;
static void register_web_handle(char* url, web_get_handle get, web_post_handle post)
{
	url_node node;
	node.url = url;
	node.get = get;
	node.post = post;
	urlNode.push_back(node);
}

static  url_node* get_url_node(char* url,int len)
{
	for (auto& node : urlNode)
	{
		if (strncmp(url, node.url, len) == 0)
		{
			return &node;
		}
	}
	return nullptr;

}


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
	if (status == 0)
		std::cout << "send success" << std::endl;

	free(req);
}


static char req_url[4096];
int url_cb(http_parser* p, const char* at, size_t length)
{
	strncpy(req_url, at, length);

	req_url[length] = 0;

	return 0;
}

static int filter_url(char* url)
{
	char req_url[4096];
	int len = 0;
	while (*url != '?' && *url != '\0')
	{
		url++;
		++len;
	}
	return len;
}

static void on_http_request(uv_stream_t* stream, char* req, int len)
{
	http_parser_settings settings;
	http_parser_settings_init(&settings);
	settings.on_url = url_cb;

	http_parser p;
	http_parser_init(&p, HTTP_REQUEST);
	http_parser_execute(&p, &settings, req, len);

	//get�ǿ���Я�������ģ�ʹ��?�ָ�url�Ͳ����������������&����
	int get_len = filter_url(req_url);
	url_node* node = get_url_node(req_url, get_len);

	if (node == NULL)
	{
		return;
	}

	switch (p.method)//���󷽷�
	{
	case HTTP_GET:
		if (node->get != NULL)
		{
			node->get(stream, req_url);
		}
		break;
	case HTTP_POST:
		if (node->post != NULL)
		{
			node->post(stream, req_url);
		}
		break;
	}
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

	//std::cout << "recv" << nread << ":" << buf->base << std::endl;
	//����
	on_http_request(stream, buf->base, buf->len);



	
}

static void connection_cb(uv_stream_t* server, int status)
{
	std::cout << "new client coming" << std::endl;

	//����ͻ���
	uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
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

char response[] = "HTTP/1.1 %d %s\r\n"\
"transfer-encoding:%s\r\n"\
"content-length:%d\r\n\r\n";

void test_get(uv_stream_t* stream, char* url)
{
	std::cout << "test_get:" << url << std::endl;

	//д����
	char data[4096];
	sprintf(data, response, 200, "OK", "identity",strlen("SUCCESS TEST_GET"));
	strcat(data, "SUCCESS TEST_GET");
	uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
	uv_buf_t send_buf = uv_buf_init(data, sizeof(data));
	uv_write(req, stream, &send_buf, 1, tcp_write_cb);
}
void test_post(uv_stream_t* stream, char* url)
{
	std::cout << "test_post" << std::endl;
	//д����
	char* data;
	sprintf(data, response, 200, "OK", "identity", strlen("SUCCESS TEST_POST"));
	strcat(data, "SUCCESS TEST_POST");
	uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
	uv_buf_t send_buf = uv_buf_init(data, sizeof(data));
	uv_write(req, stream, &send_buf, 1, tcp_write_cb);
}

void http_server()
{
	/*ע��web������*/
	register_web_handle((char*)"/test", test_get, NULL);
	register_web_handle((char*)"/test", NULL, test_post);

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