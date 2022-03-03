#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define NO_QFORKIMPL//必须添加才能使用
#include <Win32_Interop/win32fixes.h>
//#pragma comment(lib,"hiredis.lib")
//#pragma comment(lib,"Win32_Interop.lib")
#endif
#include "uv.h"
#include "redis_wrapper.h"
#include "../utils/logger.h"

#define my_alloc malloc
#define my_free free


struct connect_req
{
	char* ip;
	int port;
	char* error;//错误标识
	void* Conn;//连接对象

	void(*open_cb)(const char* err, void* Conn, void* udata);
	void* udata;

};

struct query_req
{
	void* Conn;
	char* cmd;
	void(*query_cb)(const char* err, redisReply* result, void* udata);
	void* udata;
	char* error;
	redisReply* result;
};

struct redis_Conn
{
	void* Conn;
	uv_mutex_t lock;

	bool is_closed;
};


extern "C"
{
	//执行任务
	static void connect_work(uv_work_t* work)
	{
		connect_req* req = (connect_req*)work->data;

		struct timeval timeout = { 5, 0 }; // 5 seconds
		redisContext* rc = redisConnectWithTimeout((char*)req->ip, req->port, timeout);

		if (rc->err)
		{
			log_error("redis Connection error:%s",rc->errstr);
			req->error = strdup(rc->errstr);
			req->Conn = NULL;
			redisFree(rc);
		}
		else
		{
			struct redis_Conn* c = (struct redis_Conn*)my_alloc(sizeof(struct redis_Conn));
			memset(c, 0, sizeof(struct redis_Conn));
			c->Conn = rc;
			uv_mutex_init(&c->lock);
			req->error = NULL;
			req->Conn = c;
		}
	}

	//完成任务
	static void on_connect_complete(uv_work_t* work, int status)
	{
		connect_req* req = (connect_req*)work->data;
		if (req->error)
		{
			req->open_cb(req->error, req->Conn, req->udata);
		}
		else
		{
			req->open_cb(NULL, req->Conn, req->udata);
		}

		if (req->ip)
		{
			free(req->ip);
		}

		if (req->error)
		{
			free(req->error);
		}

		my_free(req);
		my_free(work);
	}

	static void close_work(uv_work_t* work)
	{
		struct redis_Conn* req = (struct redis_Conn*)work->data;

		uv_mutex_lock(&req->lock);//加锁
		redisContext* c = (redisContext*)req->Conn;
		redisFree(c);
		req->Conn = NULL;
		uv_mutex_unlock(&req->lock);//解锁
	}

	static void on_close_complete(uv_work_t* work, int status)
	{
		struct mysql_context* req = (struct mysql_context*)work->data;
		my_free(work);
		my_free(req);
	}

	static void query_work(uv_work_t* work)
	{

		query_req* req = (query_req*)work->data;
		redis_Conn* redis_conn = (redis_Conn*)req->Conn;
		redisContext* rc = (redisContext*)redis_conn->Conn;

		uv_mutex_lock(&redis_conn->lock);//加锁

		redisReply* reply = (redisReply*)redisCommand(rc, req->cmd);
		if (reply->type == REDIS_REPLY_ERROR) //返回类型
		{
			req->error = strdup(reply->str);
			req->result = NULL;
			freeReplyObject(reply);
		}
		else 
		{
			req->result = reply;
			req->error = NULL;
		}
		uv_mutex_unlock(&redis_conn->lock);//解锁
	}

	static void on_query_complete(uv_work_t* work, int status)
	{
		query_req* req = (query_req*)work->data;
		req->query_cb(req->error, req->result, req->udata);

		if (req->cmd)
		{
			free(req->cmd);
		}
		if (req->result)
		{
			freeReplyObject(req->result);
			req->result = nullptr;
		}
		if (req->error)
		{
			free(req->error);
		}

		my_free(req);
		my_free(work);
	}
}


void redis_wrapper::connect(char* ip, int port, void(*open_cb)(const char* err, void* Conn, void* udata), void* udata /*= NULL*/)
{
	uv_work_t* work = (uv_work_t*)my_alloc(sizeof(uv_work_t));
	memset(work, 0x00, sizeof(uv_work_t));

	connect_req* req = (connect_req*)my_alloc(sizeof(connect_req));
	memset(req, 0x00, sizeof(connect_req));


	req->ip = strdup(ip);
	req->port = port;
	req->open_cb = open_cb;
	req->udata = udata;

	work->data = (connect_req*)req;

	uv_queue_work(uv_default_loop(), work, connect_work, on_connect_complete);
}

#pragma push_macro("close")
#undef close
void redis_wrapper::close(void* Conn)
#pragma pop_macro("close")
{
	redis_Conn* conn = (redis_Conn*)Conn;
	if (conn->is_closed)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)my_alloc(sizeof(uv_work_t));
	memset(work, 0x00, sizeof(uv_work_t));

	conn->is_closed = true;
	work->data = Conn;


	uv_queue_work(uv_default_loop(), work, close_work, on_close_complete);
}

void redis_wrapper::query(void* Conn, char* cmd, void(*query_cb)(const char* err, redisReply* result, void* udata), void* udata /*= NULL*/)
{
	redis_Conn* conn = (redis_Conn*)Conn;
	if (conn->is_closed)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)my_alloc(sizeof(uv_work_t));
	memset(work, 0x00, sizeof(uv_work_t));

	query_req* req = (query_req*)my_alloc(sizeof(query_req));
	memset(req, 0x00, sizeof(query_req));

	req->Conn = Conn;
	req->cmd = strdup(cmd);
	req->udata = udata;
	req->query_cb = query_cb;

	work->data = req;

	uv_queue_work(uv_default_loop(), work, query_work, on_query_complete);
}
