#pragma once

#include "NetworkDefine.h"

#define MAX_RECVBUF 8192
#define MAX_SENDBUF 8192

// Ŭ���̾�Ʈ ���� Ŭ����
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
	int mIndex = 0;								// Ŭ���̾�Ʈ �ε���
	SOCKET mSocket;								// Ŭ���̾�Ʈ ����
	stOverlappedEx m_stRecvOverlapeedEx;		// �۽� Overlapped Ȯ�� ����ü
	stOverlappedEx m_stSendOverlappedEx;		// ���� Overlapped Ȯ�� ����ü

	char mRecvBuf[MAX_RECVBUF];					// ���� ����
	char mSendBuf[MAX_SENDBUF];					// �۽� ����
};

