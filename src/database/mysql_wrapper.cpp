#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uv.h>
#include "mysql_wrapper.h"

#ifdef WIN32
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libmysql.lib")
#endif

#define my_alloc malloc
#define my_free free

struct connect_req
{
	char* ip;
	int port;
	char* db_name;//数据库名
	char* uname;//用户名
	char* pwd;//密码
	char* error;//错误标识
	void* Conn;//连接对象

	void(*open_cb)(const char* err, void* Conn, void* udata);
	void* udata;

};

struct query_req
{
	void* Conn;
	char* sql;
	void(*query_cb)(const char* err, MYSQL_RES* result, void* udata);
	void* udata;
	char* error;
	MYSQL_RES* result;
};

struct mysql_Conn
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

		MYSQL* pConn = mysql_init(NULL);

		if (mysql_real_connect(pConn, req->ip, req->uname, req->pwd, req->db_name, req->port, NULL, 0))
		{
			//req->Conn = pConn;
			mysql_Conn* mc = (mysql_Conn*)my_alloc(sizeof(mysql_Conn));
			memset(mc, 0x00, sizeof(mysql_Conn));
			mc->Conn = pConn;
			uv_mutex_init(&mc->lock);
			req->Conn = mc;
			req->error = NULL;
		}
		else
		{
			req->Conn = nullptr;
			req->error = strdup(mysql_error(pConn));
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
		if (req->uname)
		{
			free(req->uname);
		}
		if (req->pwd)
		{
			free(req->pwd);
		}
		if (req->error)
		{
			free(req->error);
		}
		if (req->db_name)
		{
			free(req->db_name);
		}

		my_free(req);
		my_free(work);
	}

	static void close_work(uv_work_t* work)
	{
		mysql_Conn* mc = (mysql_Conn*)work->data;
		uv_mutex_lock(&mc->lock);//加锁
		MYSQL* pConn = (MYSQL*)mc->Conn;

		mysql_close(pConn);
		uv_mutex_unlock(&mc->lock);//解锁
	}

	static void on_close_complete(uv_work_t* work, int status)
	{
		mysql_Conn* conn = (mysql_Conn*)work->data;
		my_free(conn);
		my_free(work);
	}

	static void query_work(uv_work_t* work)
	{

		query_req* req = (query_req*)work->data;
		mysql_Conn* mysql_conn = (mysql_Conn*)req->Conn;

		uv_mutex_lock(&mysql_conn->lock);//加锁
		MYSQL* pConn = (MYSQL*)mysql_conn->Conn;

		int ret = mysql_query(pConn, req->sql);
		if (ret != 0)
		{
			req->error = strdup(mysql_error(pConn));
			req->result = NULL;
			uv_mutex_unlock(&mysql_conn->lock);//解锁
			return;
		}

		req->error = NULL;
		
		MYSQL_RES* result = mysql_store_result(pConn);
		if (!result)
		{
			req->result = NULL;
			return;
		}
		req->result = result;
		//req->result = new std::vector<std::vector<std::string>>;
		//std::vector<std::string> temp;

		//MYSQL_ROW row;
		//int line_num = mysql_num_fields(result);//查询结果的列数
		//while (row = mysql_fetch_row(result))
		//{
		//	for (int i = 0; i < line_num; ++i)
		//	{
		//		temp.emplace_back(row[i]);
		//	}
		//	req->result->emplace_back(temp);
		//	temp.clear();
		//}

		//mysql_free_result(result);
		uv_mutex_unlock(&mysql_conn->lock);//解锁
	}

	static void on_query_complete(uv_work_t* work, int status)
	{
		query_req* req = (query_req*)work->data;
		req->query_cb(req->error, req->result, req->udata);

		if (req->sql)
		{
			free(req->sql);
		}
		if (req->result)
		{
			mysql_free_result(req->result);
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

void mysql_wrapper::connect(char* ip, int port, char* db_name, char* uname, char* pwd, void(*open_cb)(const char* err, void* Conn, void* udata), void* udata /*= NULL*/)
{
	uv_work_t* work =(uv_work_t*)my_alloc(sizeof(uv_work_t));
	memset(work, 0x00, sizeof(uv_work_t));

	connect_req* req = (connect_req*)my_alloc(sizeof(connect_req));
	memset(req, 0x00, sizeof(connect_req));
	

	req->ip = strdup(ip);
	req->port = port;
	req->db_name = strdup(db_name);
	req->uname = strdup(uname);
	req->pwd = strdup(pwd);
	req->open_cb = open_cb;
	req->udata = udata;

	work->data = (connect_req*)req;

	uv_queue_work(uv_default_loop(), work, connect_work, on_connect_complete);
}

void mysql_wrapper::close(void* Conn)
{
	mysql_Conn* conn = (mysql_Conn*)Conn;
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

void mysql_wrapper::query(void* Conn, char* sql, void(*query_cb)(const char* err, MYSQL_RES* result, void* udata), void* udata /*= NULL*/)
{
	mysql_Conn* conn = (mysql_Conn*)Conn;
	if (conn->is_closed)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)my_alloc(sizeof(uv_work_t));
	memset(work, 0x00, sizeof(uv_work_t));

	query_req* req = (query_req*)my_alloc(sizeof(query_req));
	memset(req, 0x00, sizeof(query_req));

	req->Conn = Conn;
	req->sql = strdup(sql);
	req->udata = udata;
	req->query_cb = query_cb;

	work->data = req;
	
	uv_queue_work(uv_default_loop(), work, query_work, on_query_complete);
}
