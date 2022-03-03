#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <winsock.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libmysql.lib")

#include "mysql.h"

int sql()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	//创建mysql连接
	MYSQL* pConn = mysql_init(NULL);

	mysql_real_connect(pConn, "127.0.0.1", "root", "20171028", "test", 3306, NULL, 0);

	mysql_query(pConn, "set names utf8");//设置编码
	//插入一条记录
	//int ret = mysql_query(pConn, "insert into student(id,name,chinese,english,math) values(11,'张三',100,100,100);");
	//if (ret != 0)
	//{
	//	printf("%s", mysql_error(pConn));//mysql_error(pConn)返回错误信息
	//}
	//int line = mysql_affected_rows(pConn);//返回受影响的行数
	//printf("%d", line);
	//修改一条记录 

	//查询一条记录
	int ret = mysql_query(pConn, "select * from student;");
	if (ret != 0)
	{
		printf("%s", mysql_error(pConn));//mysql_error(pConn)返回错误信息
	}
	else//获得查询结果
	{
		MYSQL_RES* result = mysql_store_result(pConn);
		MYSQL_ROW row;
		while (row = mysql_fetch_row(result))
		{
			printf("%s,%s,%s,%s\n", row[0], row[1], row[2], row[3]);
		}
	}

	//关闭mysql连接
	mysql_close(pConn);

	system("pause");
	return 0;
}