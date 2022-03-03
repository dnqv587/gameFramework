#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#ifdef __cplusplus
extern "C" {
#endif

	// 获取当前的时间戳
	unsigned long timestamp();

	// 获取给定日期的时间戳"%Y(年)%m(月)%d(日)%H(小时)%M(分)%S(秒)"
	//日期格式转时间戳格式
	/*
	fmt_date：日期格式
	date：日期
	返回：时间戳
	*/
	unsigned long date2timestamp(const char* fmt_date, const char* date);

	// fmt_date "%Y(年)%m(月)%d(日)%H(小时)%M(分)%S(秒)"
	//时间戳格式转日期格式
	/*
	t：时间戳
	fmt_date：日期格式
	out_buf：日期---传出
	buf_len：长度
	*/
	void timestamp2date(unsigned long t, char* fmt_date, char* out_buf, int buf_len);

	//今天的时间戳
	unsigned long timestamp_today();
	//昨天的时间戳
	unsigned long timestamp_yesterday();
	//今天的日期
	/*
	fmt_date：日期格式
	out_buf：日期--传出
	buf_len：长度
	*/
	void date_today(char* fmt_date, char* out_buf, int buf_len);


#ifdef __cplusplus
}
#endif

#endif

