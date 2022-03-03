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

	//����mysql����
	MYSQL* pConn = mysql_init(NULL);

	mysql_real_connect(pConn, "127.0.0.1", "root", "20171028", "test", 3306, NULL, 0);

	mysql_query(pConn, "set names utf8");//���ñ���
	//����һ����¼
	//int ret = mysql_query(pConn, "insert into student(id,name,chinese,english,math) values(11,'����',100,100,100);");
	//if (ret != 0)
	//{
	//	printf("%s", mysql_error(pConn));//mysql_error(pConn)���ش�����Ϣ
	//}
	//int line = mysql_affected_rows(pConn);//������Ӱ�������
	//printf("%d", line);
	//�޸�һ����¼ 

	//��ѯһ����¼
	int ret = mysql_query(pConn, "select * from student;");
	if (ret != 0)
	{
		printf("%s", mysql_error(pConn));//mysql_error(pConn)���ش�����Ϣ
	}
	else//��ò�ѯ���
	{
		MYSQL_RES* result = mysql_store_result(pConn);
		MYSQL_ROW row;
		while (row = mysql_fetch_row(result))
		{
			printf("%s,%s,%s,%s\n", row[0], row[1], row[2], row[3]);
		}
	}

	//�ر�mysql����
	mysql_close(pConn);

	system("pause");
	return 0;
}