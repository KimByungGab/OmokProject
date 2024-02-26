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

// 송수신 확인
enum class IOOperation
{
	RECV,
	SEND
};

// Overlapped 확장 구조체
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;	// Overlapped 구조체
	SOCKET m_socketClient;			// 클라이언트 소켓
	WSABUF m_wsaBuf;				// 송수신에 사용되는 WSA 버퍼
	IOOperation m_eOperation;		// 송수신 확인 용도
};