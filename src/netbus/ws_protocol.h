#pragma once
//websocket协议模块

class session;
class ws_protocol
{
public:
	//握手--建立连接
	static bool ws_shake_hand(session* s, char* body, int len);
	//读取websocket头
	/*
	参数：
	pkg_data：实际收到的数据包
	pkg_len：实际收到的数据包长度
	pkg_size：应该收到的数据包长度
	out_header_size：包头长度--传出
	*/
	static bool read_ws_header(unsigned char* pkg_data, int pkg_len, int* pkg_size,int* out_header_size);
	//解析解析数据
	/*
	参数
	raw_data：数据--传入传出
	mask：掩码--使用异或解码
	raw_len：数据长度
	*/
	static void parser_ws_recv_data(unsigned char* raw_data, unsigned char* mask,int raw_len);
	//打包发送的数据
	static unsigned char* package_ws_send_data(const unsigned char* raw_data, int len,int* ws_data_len);
	//释放发送数据的内存
	static void free_ws_send_pkg(unsigned char* ws_pkg);

private:


};
