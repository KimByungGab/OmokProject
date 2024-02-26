#pragma once

#include "NetworkDefine.h"

#define MAX_RECVBUF 8192
#define MAX_SENDBUF 8192

// 클라이언트 정보 클래스
class ClientInfo
{
public:
	ClientInfo();
	int GetIndex();
	void SetIndex(const int index);
	bool IsConnected();
	SOCKET GetSocket();
	void SetSocket(SOCKET socket);
	char* GetRecvBuffer();
	char* GetSendBuffer();
	void Clear();

private:
	void Init();

public:
	int mIndex = 0;								// 클라이언트 인덱스
	SOCKET mSocket;								// 클라이언트 소켓
	stOverlappedEx m_stRecvOverlapeedEx;		// 송신 Overlapped 확장 구조체
	stOverlappedEx m_stSendOverlappedEx;		// 수신 Overlapped 확장 구조체

	char mRecvBuf[MAX_RECVBUF];					// 수신 버퍼
	char mSendBuf[MAX_SENDBUF];					// 송신 버퍼
};

