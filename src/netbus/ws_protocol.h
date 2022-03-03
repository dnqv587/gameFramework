#pragma once
//websocketЭ��ģ��

class session;
class ws_protocol
{
public:
	//����--��������
	static bool ws_shake_hand(session* s, char* body, int len);
	//��ȡwebsocketͷ
	/*
	������
	pkg_data��ʵ���յ������ݰ�
	pkg_len��ʵ���յ������ݰ�����
	pkg_size��Ӧ���յ������ݰ�����
	out_header_size����ͷ����--����
	*/
	static bool read_ws_header(unsigned char* pkg_data, int pkg_len, int* pkg_size,int* out_header_size);
	//������������
	/*
	����
	raw_data������--���봫��
	mask������--ʹ��������
	raw_len�����ݳ���
	*/
	static void parser_ws_recv_data(unsigned char* raw_data, unsigned char* mask,int raw_len);
	//������͵�����
	static unsigned char* package_ws_send_data(const unsigned char* raw_data, int len,int* ws_data_len);
	//�ͷŷ������ݵ��ڴ�
	static void free_ws_send_pkg(unsigned char* ws_pkg);

private:


};
