#pragma once

#include "NetworkDefine.h"
#include "ClientInfo.h"
#include "Packet.h"
#include "PacketManager.h"

#define SERVER_PORT 12345
#define MAX_CLIENT 100

// ��Ʈ��ũ�� �ھ� Ŭ����
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
	vector<ClientInfo*> mClientInfos;				// Ŭ���̾�Ʈ ���� ������ vector (���� ����: ���Ի��� ���� �����鼭 �ڿ� �߰��� �� ���̱� ������ ���� ������ ���� vector�� ����)
	SOCKET mListenSocket;							// Listen ����
	vector<thread> mIOWorkerThreads;				// IO �۾� ������ vector
	bool mIsWorkerRun = false;						// IO �۾� ������ �۵� ����
	thread mAcceptThread;							// Accept ������
	bool mIsAcceptRun = false;						// Accept ������ �۵� ����
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;		// IOCP �ڵ�
	int mMaxWorkerThread;							// �ִ� �۾� ������ ����

	unique_ptr<PacketManager> m_pPacketManager;		// ��Ŷ �Ŵ��� ����ũ ������
};