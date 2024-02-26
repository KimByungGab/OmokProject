#include "ClientInfo.h"

// 생성자
ClientInfo::ClientInfo()
{
	// 초기화
	Init();
}

/*
* @brief 클라이언트 인덱스 호출
* @author 김병갑
* @return 인덱스
*/
int ClientInfo::GetIndex()
{
	return mIndex;
}

/*
* @brief 클라이언트 인덱스 세팅
* @author 김병갑
* @param index: 클라이언트 인덱스
* @return void
*/
void ClientInfo::SetIndex(const int index)
{
	mIndex = index;
}

/*
* @brief 클라이언트 연결 여부
* @author 김병갑
* @return 연결 여부. true면 연결, false면 연결안됨
*/
bool ClientInfo::IsConnected()
{
	return mSocket != INVALID_SOCKET;
}

/*
* @brief 클라이언트 소켓 호출
* @author 김병갑
* @return 소켓
*/
SOCKET ClientInfo::GetSocket()
{
	return mSocket;
}

/*
* @brief 클라이언트 소켓 세팅
* @author 김병갑
* @param socket: 소켓
* @return void
*/
void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

/*
* @brief 수신 버퍼 호출
* @author 김병갑
* @return 수신 버퍼 포인터
*/
char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}

/*
* @brief 송신 버퍼 호출
* @author 김병갑
* @return 송신 버퍼 포인터
*/
char* ClientInfo::GetSendBuffer()
{
	return mSendBuf;
}

/*
* @brief 클라이언트 정보 클리어
* @author 김병갑
* @return void
*/
void ClientInfo::Clear()
{
	Init();
}

/*
* @brief 클라이언트 초기화
* @author 김병갑
* @return void
*/
void ClientInfo::Init()
{
	// 수신 및 송신 Overlapped 확장 구조체 초기화
	memset(&m_stRecvOverlapeedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));

	// 소켓 초기화
	mSocket = INVALID_SOCKET;

	// 수신 및 송신 버퍼 초기화
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
}
