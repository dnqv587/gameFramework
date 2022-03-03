#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//windos下的头文件和库
#ifdef WIN32
#include <WInSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment (lib,"WSOCK32.LIB")
int sock();
#endif