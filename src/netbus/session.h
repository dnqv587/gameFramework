#pragma once

struct cmd_msg;
struct raw_msg;
class session
{
public:
	//�رսӿ�
	virtual void close() = 0;
	//��������
	virtual void send_data(unsigned char* body, int len) = 0;
	//����cmd_msg
	virtual void send_msg(cmd_msg* msg) = 0;
	virtual void send_raw_cmd(raw_msg* msg) = 0;
	//��ȡ��ַ
	virtual const char* get_address(int* client_port) = 0;


	unsigned int as_client = 0;
	unsigned int utag = 0;
	unsigned int uid = 0;
private:
};
