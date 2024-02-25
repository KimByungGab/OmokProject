#include "ClientInfo.h"

ClientInfo::ClientInfo()
{
	Init();
}

int ClientInfo::GetIndex()
{
	return mIndex;
}

void ClientInfo::SetIndex(const int index)
{
	mIndex = index;
}

bool ClientInfo::IsConnected()
{
	return mSocket != INVALID_SOCKET;
}

SOCKET ClientInfo::GetSocket()
{
	return mSocket;
}

void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}

char* ClientInfo::GetSendBuffer()
{
	return mSendBuf;
}

void ClientInfo::Clear()
{
	Init();
}

void ClientInfo::Init()
{
	memset(&m_stRecvOverlapeedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));

	mSocket = INVALID_SOCKET;

	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
}
