#pragma once

#ifdef __cplusplus
extern "C" {
#endif
	struct timer;
	/*
	ѭ��schedule
	������
	on_timer���ص���������timer������ʱ�����;
	udata: �ص���������
	after_sec: �����뿪ʼִ��;
	repeat_count: ִ�ж��ٴ�, repeat_count == -1һֱִ��;
	repeat_mesc��ѭ�����ʱ�䣬��λ΢�� 1000=1��
	����timer�ľ��;
	*/
	struct timer* schedule_repeat(void(*on_timer)(void* udata), void* udata, int after_msec, int repeat_count, int repeat_mesc);
	//��ȡschedule�Ĳ���
	void* get_timer_udata(struct timer* t);

	// ȡ�������timer;
	void cancel_timer(struct timer* t);

	struct timer* schedule_once(void(*on_timer)(void* udata), void* udata, int after_msec);


#ifdef __cplusplus
}
#endif

