#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#ifdef __cplusplus
extern "C" {
#endif

	// ��ȡ��ǰ��ʱ���
	unsigned long timestamp();

	// ��ȡ�������ڵ�ʱ���"%Y(��)%m(��)%d(��)%H(Сʱ)%M(��)%S(��)"
	//���ڸ�ʽתʱ�����ʽ
	/*
	fmt_date�����ڸ�ʽ
	date������
	���أ�ʱ���
	*/
	unsigned long date2timestamp(const char* fmt_date, const char* date);

	// fmt_date "%Y(��)%m(��)%d(��)%H(Сʱ)%M(��)%S(��)"
	//ʱ�����ʽת���ڸ�ʽ
	/*
	t��ʱ���
	fmt_date�����ڸ�ʽ
	out_buf������---����
	buf_len������
	*/
	void timestamp2date(unsigned long t, char* fmt_date, char* out_buf, int buf_len);

	//�����ʱ���
	unsigned long timestamp_today();
	//�����ʱ���
	unsigned long timestamp_yesterday();
	//���������
	/*
	fmt_date�����ڸ�ʽ
	out_buf������--����
	buf_len������
	*/
	void date_today(char* fmt_date, char* out_buf, int buf_len);


#ifdef __cplusplus
}
#endif

#endif

