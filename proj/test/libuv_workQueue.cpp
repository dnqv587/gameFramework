#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "libuv.h"


/*工作队列的用处：
1：复杂的算法放到工作队列
2：IO和获取数据库结果
...
*/

//在另外的线程里面调用
void work_cb(uv_work_t* req)
{
	std::cout << "data:" << (int)req->data << std::endl;
	printf("work_cb thread id 0x%d\n", (int)uv_thread_self());

}

//当工作队列里面的线程执行完成任务后，通知主线程：
//主线程调用此函数
void after_work_cb(uv_work_t* req, int status)
{
	printf("after_work_cb thread id 0x%d\n", (int)uv_thread_self());
}

void workQueue()
{
	uv_work_t uv_work;
	uv_work.data = (void*)6;

	printf("workQueue thread id 0x%d\n", (int)uv_thread_self());
	uv_queue_work(uv_default_loop(), &uv_work, work_cb, after_work_cb);//添加工作队列


	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}