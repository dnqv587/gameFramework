#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "libuv.h"


/*�������е��ô���
1�����ӵ��㷨�ŵ���������
2��IO�ͻ�ȡ���ݿ���
...
*/

//��������߳��������
void work_cb(uv_work_t* req)
{
	std::cout << "data:" << (int)req->data << std::endl;
	printf("work_cb thread id 0x%d\n", (int)uv_thread_self());

}

//����������������߳�ִ����������֪ͨ���̣߳�
//���̵߳��ô˺���
void after_work_cb(uv_work_t* req, int status)
{
	printf("after_work_cb thread id 0x%d\n", (int)uv_thread_self());
}

void workQueue()
{
	uv_work_t uv_work;
	uv_work.data = (void*)6;

	printf("workQueue thread id 0x%d\n", (int)uv_thread_self());
	uv_queue_work(uv_default_loop(), &uv_work, work_cb, after_work_cb);//��ӹ�������


	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}