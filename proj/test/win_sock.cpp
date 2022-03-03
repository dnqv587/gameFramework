#include "win_sock.h"
#include "../../src/netbus/tcp_protocol.h"
#include "../../src/apps/test/proto/game.pb.h"
#include "../../src/netbus/netbus.h"

void service()
{
	//����socket
	int lfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//TCP
	if (lfd == INVALID_SOCKET)
	{
		perror("socket error");
	}
	struct sockaddr_in serv;
	memset(&serv, 0x00, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(8888);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	//��socket
	int ret = bind(lfd, (struct sockaddr*)&serv, sizeof(serv));
	if (ret < 0)
	{
		perror("bind error");
	}

	//����socket
	listen(lfd, 128);
	struct sockaddr_in client;
	int len = sizeof(client);
	//���������ӣ������ͨ���ļ�������
	int cfd = accept(lfd, (struct sockaddr*)&client, &len);
	int i = 0;
	int n;
	char buf[1024];
	//��д����
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		n = recv(cfd, buf, sizeof(buf), 0);
		if (n <= 0)
		{
			printf("read error");
			break;
		}
		printf("%s", buf);
		for (i = 0; i < n; i++)
		{
			buf[i] = toupper(buf[i]);
		}

		send(cfd, buf, n, 0);
	}

	closesocket(lfd);
	closesocket(cfd);
}

void client()
{
	//����socket����ü����ļ�������
	int cfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//TCP
	if (cfd == INVALID_SOCKET)
	{
		perror("socket error");
	}
	//���ӷ����
	struct sockaddr_in addr;
	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	//inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.S_un.S_addr);//WS2tcpip.h
	memcpy(&addr.sin_addr, "127.0.0.1", sizeof("127.0.0.1"));
	int ret = connect(cfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		perror("connect error");
	}
	char buf[1024];
	while (1)
	{
		memset(buf, 0, sizeof(buf));//��ջ�����
		//����׼��������
		int n = send(cfd, "hello", 5, 0);
		if (n <= 0)
		{
			printf("read error");
			break;
		}
		//д����
		recv(cfd, buf, sizeof(buf), 0);
		printf("read=%s\n", buf);
	}
	closesocket(cfd);
}

void test()
{
	//����socket����ü����ļ�������
	int cfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//TCP
	if (cfd == INVALID_SOCKET)
	{
		perror("socket error");
	}
	//���ӷ����
	struct sockaddr_in addr;
	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	//inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.S_un.S_addr);//WS2tcpip.h
	//memcpy(&addr.sin_addr, "127.0.0.1", sizeof("127.0.0.1"));
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(cfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		perror("connect error");
	}
	
	LoginReq req;
	req.set_age(10);
	req.set_name("dai");
	req.set_email("1336047549@qq.com");

	int len = req.ByteSizeLong();
	char* data = (char*)malloc(8 + len);
	memset(data, 0x00, 8 + len);
	req.SerializePartialToArray(data + 8, len);

	int pkg_len;
	netbus* test = netbus::instance();
	test->init();
	unsigned char* pkg_data = tcp_protocol::package((unsigned char*)data, 8 + len, &pkg_len);
	send(cfd, (const char*)pkg_data, pkg_len, 0);
	free(data);

	tcp_protocol::release_package(pkg_data);

	unsigned char recv_buf[256];
	int recv_len = recv(cfd, (char*)recv_buf, 256, 0);

	int pkg_size, header_size;
	tcp_protocol::read_header(recv_buf, recv_len, &pkg_size, &header_size);
	
	req.ParseFromArray(recv_buf + header_size + 8, pkg_size - header_size - 8);
	printf("%s:%d\n", req.name().c_str(), req.age());

	closesocket(cfd);
	
}

int sock()
{

	//service();
	//client();
	test();
	return 0;
}

