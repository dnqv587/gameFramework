#pragma once

enum {
	DEBUG = 0,
	WARNING,
	LOG_ERROR,
};

#define log_debug(msg, ...) logger::log(__FILE__, __LINE__, DEBUG, msg, ## __VA_ARGS__);
#define log_warning(msg, ...) logger::log(__FILE__, __LINE__, WARNING, msg, ## __VA_ARGS__);
#define log_error(msg, ...) logger::log(__FILE__, __LINE__, LOG_ERROR, msg, ## __VA_ARGS__);

class logger {
public:
	/*
	��ʼ��
	������
	path�������ַ
	file_name:�ļ���
	std_output����׼�������--Ĭ�Ϲر�
	*/
	static void init(const char* path, const char* file_name, bool std_output = false);

	static void log(const char* file_name, int line_num, int level, const char* msg, ...);
};



