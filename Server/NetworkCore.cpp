#include "NetworkCore.h"

NetworkCore::~NetworkCore()
{
	WSACleanup();
}

bool NetworkCore::InitSocket()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup() error: " << WSAGetLastError() << endl;
		return false;
	}

	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (mListenSocket == INVALID_SOCKET)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "소켓 초기화 성공" << endl;
	return true;
}

bool NetworkCore::BindAndListen()
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int result = ::bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (result != 0)
	{
		cout << "bind() error: " << WSAGetLastError() << endl;
		return false;
	}

	result = listen(mListenSocket, SOMAXCONN);
	if (result != 0)
	{
		cout << "listen() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "서버 등록 성공" << endl;
	return true;
}

bool NetworkCore::StartServer()
{
	m_pPacketManager = make_unique<PacketManager>();
	m_pPacketManager->SendPacketFunc = [&](UINT32 clientIndex, UINT16 packetSize, char* pSendPacket)
		{
			SendMsg(GetClientInfo(clientIndex), pSendPacket, packetSize);
		};
	m_pPacketManager->Init(MAX_CLIENT);
	m_pPacketManager->Run();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	mMaxWorkerThread = sysInfo.dwNumberOfProcessors;

	CreateClient(MAX_CLIENT);

	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	if (CreateWorkerThread() == false)
		return false;

	if (CreateAcceptThread() == false)
		return false;

	cout << "서버 시작" << endl;

	return true;
}

void NetworkCore::EndServer()
{
	mIsAcceptRun = false;
	if (mAcceptThread.joinable())
		mAcceptThread.join();

	CloseHandle(mIOCPHandle);

	mIsWorkerRun = false;
	for (auto& t : mIOWorkerThreads)
	{
		if (t.joinable())
			t.join();
	}

	closesocket(mListenSocket);
}

void NetworkCore::CreateClient(const UINT32 maxClientCount)
{
	int currentSize = mClientInfos.size();
	int maxSize = currentSize + maxClientCount;
	for (UINT32 i = currentSize; i < maxSize; i++)
	{
		ClientInfo* client = new ClientInfo();
		client->SetIndex(i);

		mClientInfos.push_back(client);
	}

	m_pPacketManager->AddSession(maxClientCount);
}

ClientInfo* NetworkCore::GetEmptyClientInfo()
{
	for (auto& client : mClientInfos)
	{
		if (client->GetSocket() == INVALID_SOCKET)
			return client;
	}

	return nullptr;
}

ClientInfo* NetworkCore::GetClientInfo(const UINT32 clientIndex)
{
	return mClientInfos[clientIndex];
}

bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->GetSocket(), mIOCPHandle, (ULONG_PTR)pClientInfo, 0);
	if (hIOCP == NULL || mIOCPHandle != hIOCP)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

bool NetworkCore::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	pClientInfo->m_stRecvOverlapeedEx.m_wsaBuf.len = MAX_RECVBUF;
	pClientInfo->m_stRecvOverlapeedEx.m_wsaBuf.buf = pClientInfo->GetRecvBuffer();
	pClientInfo->m_stRecvOverlapeedEx.m_eOperation = IOOperation::RECV;

	int result = WSARecv(pClientInfo->GetSocket(),
		&(pClientInfo->m_stRecvOverlapeedEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (pClientInfo->m_stRecvOverlapeedEx),
		NULL);

	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSARecv() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

bool NetworkCore::SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen)
{
	DWORD dwSendNumBytes = 0;

	memcpy(pClientInfo->mSendBuf, pMsg, nLen);

	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->GetSendBuffer();
	pClientInfo->m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

	int result = WSASend(pClientInfo->GetSocket(),
		&(pClientInfo->m_stSendOverlappedEx.m_wsaBuf),
		1,
		&dwSendNumBytes,
		0,
		(LPWSAOVERLAPPED) & (pClientInfo->m_stSendOverlappedEx),
		NULL);

	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSASend() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

bool NetworkCore::CreateWorkerThread()
{
	mIsWorkerRun = true;
	for (int i = 0; i < mMaxWorkerThread; i++)
	{
		mIOWorkerThreads.push_back(thread([this]()
			{
				WorkerThread();
			}));
	}

	cout << "WorkerThread 시작" << endl;

	return true;
}

void NetworkCore::WorkerThread()
{
	ClientInfo* pClientInfo = nullptr;
	BOOL bSuccess = TRUE;
	DWORD dwIoSize = 0;
	LPOVERLAPPED lpOverlapped = nullptr;

	while (mIsWorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo,
			&lpOverlapped, INFINITE);

		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == nullptr)
		{
			mIsWorkerRun = false;
			continue;
		}

		if (lpOverlapped == nullptr)
		{
			continue;
		}

		if (bSuccess == FALSE || (bSuccess == TRUE && dwIoSize == 0))
		{
			cout << "클라 접속 종료! " << (int)pClientInfo->GetSocket() << endl;
			CloseSocket(pClientInfo);

			continue;
		}

		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		if (pOverlappedEx->m_eOperation == IOOperation::RECV)
		{
			m_pPacketManager->ReceivePacket(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuffer());

			BindRecv(pClientInfo);
		}
		else if (pOverlappedEx->m_eOperation == IOOperation::SEND)
		{
			cout << "[송신] bytes: " << dwIoSize << endl;
		}
		else
		{
			cout << "socket(" << (int)pClientInfo->GetSocket() << ")에서 예외상황" << endl;
			break;
		}
	}
}

bool NetworkCore::CreateAcceptThread()
{
	mIsAcceptRun = true;
	mAcceptThread = thread([this]()
		{
			AcceptThread();
		});

	cout << "AcceptThread 시작" << endl;

	return true;
}

void NetworkCore::AcceptThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (mIsAcceptRun)
	{
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == nullptr)
		{
			CreateClient(MAX_CLIENT);
			continue;
		}

		SOCKET newSocket = accept(mListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (newSocket == INVALID_SOCKET)
			continue;

		pClientInfo->SetSocket(newSocket);

		if (BindIoCompletionPort(pClientInfo) == false)
		{
			cout << "AcceptThread BindIoCompletionPort Error" << endl;
			return;
		}

		if (BindRecv(pClientInfo) == false)
		{
			cout << "AcceptThread BindRecv Error" << endl;
			return;
		}

		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
		cout << "클라 접속: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->GetSocket() << ")" << endl;

		m_pPacketManager->ConnectClient(pClientInfo->GetIndex());
	}
}

void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	int clientIndex = pClientInfo->GetIndex();

	struct linger stLinger = { 0, 0 };

	if (bIsForce == true)
		stLinger.l_onoff = 1;

	shutdown(pClientInfo->GetSocket(), SD_BOTH);

	setsockopt(pClientInfo->GetSocket(), SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	closesocket(pClientInfo->GetSocket());
	pClientInfo->SetSocket(INVALID_SOCKET);

	m_pPacketManager->DisconnectClient(clientIndex);
}
