#pragma once

#include "NetworkDefine.h"
#include "ClientInfo.h"
#include "Packet.h"
#include "PacketManager.h"

#define SERVER_PORT 12345
#define MAX_CLIENT 100

// 네트워크의 코어 클래스
class NetworkCore
{
public:
	~NetworkCore();
	bool InitSocket();
	bool BindAndListen();
	bool StartServer();
	void EndServer();

private:
	void CreateClient(const UINT32 maxClientCount);
	ClientInfo* GetEmptyClientInfo();
	ClientInfo* GetClientInfo(const UINT32 clientIndex);
	bool BindIoCompletionPort(ClientInfo* pClientInfo);
	bool BindRecv(ClientInfo* pClientInfo);
	bool SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen);
	bool CreateWorkerThread();
	void WorkerThread();
	bool CreateAcceptThread();
	void AcceptThread();
	void CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);

private:
	vector<ClientInfo*> mClientInfos;				// 클라이언트 정보 포인터 vector (선정 이유: 삽입삭제 하지 않으면서 뒤에 추가만 될 것이기 때문에 가장 접근이 빠른 vector로 선정)
	SOCKET mListenSocket;							// Listen 소켓
	vector<thread> mIOWorkerThreads;				// IO 작업 쓰레드 vector
	bool mIsWorkerRun = false;						// IO 작업 쓰레드 작동 여부
	thread mAcceptThread;							// Accept 쓰레드
	bool mIsAcceptRun = false;						// Accept 쓰레드 작동 여부
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;		// IOCP 핸들
	int mMaxWorkerThread;							// 최대 작업 쓰레드 개수

	unique_ptr<PacketManager> m_pPacketManager;		// 패킷 매니저 유니크 포인터
};