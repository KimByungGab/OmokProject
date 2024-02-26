#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

using namespace std;

// �ۼ��� Ȯ��
enum class IOOperation
{
	RECV,
	SEND
};

// Overlapped Ȯ�� ����ü
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;	// Overlapped ����ü
	SOCKET m_socketClient;			// Ŭ���̾�Ʈ ����
	WSABUF m_wsaBuf;				// �ۼ��ſ� ���Ǵ� WSA ����
	IOOperation m_eOperation;		// �ۼ��� Ȯ�� �뵵
};