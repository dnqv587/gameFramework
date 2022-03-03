#pragma once 
#include <string>
#include <vector>
#include <google/protobuf/message.h>
#include <string>
#include <map>
//ͨѶЭ�����ģ��


enum
{
	PROTO_JSON = 0,//Json
	PROTO_BUF = 1//protobuf
};

//�����ʽ
struct cmd_msg
{
	int stype;//�����--2�ֽ�
	int ctype;//�����--2�ֽ�
	unsigned int utag;//�û���ʶ--4�ֽ�
	void* body;//������---protobuf��jsonЭ��
};

struct raw_msg {
	int stype;
	int ctype;
	unsigned int utag;

	unsigned char* raw_data;
	int raw_len;
};

class proto_man
{
public:
	//��ʼ��---����Э������
	/*
	proto_type��Э������--PROTO_BUF/PROTO_JSON
	
	*/
	static void init(int proto_type);
	//��������ӵ�ӳ�����
	static void register_protobuf_cmd_map(std::map<int, std::string>& map);
	//��ȡЭ������
	static int proto_type();
	/*
	��������
	������
	cmd������--�����������š�����š��û���ʶ�����԰���������
	cmd_len������--���ڵ���8
	out_msg�����ݽṹ��---����
	*/
	static bool decode_cmd_msg(unsigned char* cmd, int cmd_len, cmd_msg** out_msg);
	//���ٽ������ݵ�cmd_msg����
	static void cmd_msg_free(cmd_msg* msg);
	/*
	��������
	������
	msg����Ҫ�����cmd_msg�ṹ������
	out_len�����������ݳ���--����
	���أ�����������
	*/
	static unsigned char* encode_msg_to_raw(const cmd_msg* msg, int* out_len);
	//���ٱ������
	static void msg_raw_free(unsigned char* raw);

	//ͨ��protobuf����ģʽ����message����
	static google::protobuf::Message* create_message(const char* type_name);
	//����message����
	static void release_message(google::protobuf::Message* m);
	//���protobuf��
	static const char* protobuf_cmd_name(int ctype);

private:
	//����Э������
	static int g_proto_type ;
	//protobufӳ���
	static std::map<int, std::string> g_pb_cmd_map;



};