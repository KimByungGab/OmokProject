#pragma once

#include "NetworkDefine.h"

#define MAX_RECVBUF 8192
#define MAX_SENDBUF 8192

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
	int mIndex = 0;
	SOCKET mSocket;
	stOverlappedEx m_stRecvOverlapeedEx;
	stOverlappedEx m_stSendOverlappedEx;

	char mRecvBuf[MAX_RECVBUF];
	char mSendBuf[MAX_SENDBUF];
};

