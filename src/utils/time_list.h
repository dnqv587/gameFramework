#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	struct timer;
	/*
	循环schedule
	参数：
	on_timer：回调函数，当timer触发的时候调用;
	udata: 回调函数参数
	after_sec: 多少秒开始执行;
	repeat_count: 执行多少次, repeat_count == -1一直执行;
	repeat_mesc：循环间隔时间，单位微秒 1000=1秒
	返回timer的句柄;
	*/
	struct timer* schedule_repeat(void(*on_timer)(void* udata), void* udata, int after_msec, int repeat_count, int repeat_mesc);
	//获取schedule的参数
	void* get_timer_udata(struct timer* t);

	// 取消掉这个timer;
	void cancel_timer(struct timer* t);

	struct timer* schedule_once(void(*on_timer)(void* udata), void* udata, int after_msec);


#ifdef __cplusplus
}
#endif

