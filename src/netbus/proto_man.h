#pragma once 
#include <string>
#include <vector>
#include <google/protobuf/message.h>
#include <string>
#include <map>
//通讯协议管理模块


enum
{
	PROTO_JSON = 0,//Json
	PROTO_BUF = 1//protobuf
};

//命令格式
struct cmd_msg
{
	int stype;//服务号--2字节
	int ctype;//命令号--2字节
	unsigned int utag;//用户标识--4字节
	void* body;//数据体---protobuf或json协议
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
	//初始化---定义协议类型
	/*
	proto_type：协议类型--PROTO_BUF/PROTO_JSON
	
	*/
	static void init(int proto_type);
	//将命令添加到映射表里
	static void register_protobuf_cmd_map(std::map<int, std::string>& map);
	//获取协议类型
	static int proto_type();
	/*
	解码数据
	参数：
	cmd：数据--必须包括服务号、命令号、用户标识，可以包括数据体
	cmd_len：长度--大于等于8
	out_msg：数据结构体---传出
	*/
	static bool decode_cmd_msg(unsigned char* cmd, int cmd_len, cmd_msg** out_msg);
	//销毁解码数据的cmd_msg对象
	static void cmd_msg_free(cmd_msg* msg);
	/*
	编码数据
	参数：
	msg：需要编码的cmd_msg结构体数据
	out_len：编码后的数据长度--传出
	返回：编码后的数据
	*/
	static unsigned char* encode_msg_to_raw(const cmd_msg* msg, int* out_len);
	//销毁编码对象
	static void msg_raw_free(unsigned char* raw);

	//通过protobuf工厂模式创建message对象
	static google::protobuf::Message* create_message(const char* type_name);
	//销毁message对象
	static void release_message(google::protobuf::Message* m);
	//获得protobuf名
	static const char* protobuf_cmd_name(int ctype);

private:
	//数据协议类型
	static int g_proto_type ;
	//protobuf映射表
	static std::map<int, std::string> g_pb_cmd_map;



};