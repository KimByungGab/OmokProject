#include "NetworkCore.h"

// �Ҹ���
NetworkCore::~NetworkCore()
{
	WSACleanup();
}

/*
* @brief Listen ���� �ʱ�ȭ
* @author �躴��
* @return �ʱ�ȭ ���� ����. true�� ����, false�� ����
*/
bool NetworkCore::InitSocket()
{
	// WSAData �ʱ�ȭ
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup() error: " << WSAGetLastError() << endl;
		return false;
	}

	// Listen ���� �ʱ�ȭ
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (mListenSocket == INVALID_SOCKET)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "���� �ʱ�ȭ ����" << endl;
	return true;
}

/*
* @brief ���� bind, Listen
* @author �躴��
* @return bind�� Listen ���� ����. true�� ����, false�� ����
*/
bool NetworkCore::BindAndListen()
{
	// ���� ���� ����ü
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// ���� bind
	int result = ::bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (result != 0)
	{
		cout << "bind() error: " << WSAGetLastError() << endl;
		return false;
	}

	// ���� listen
	result = listen(mListenSocket, SOMAXCONN);
	if (result != 0)
	{
		cout << "listen() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "���� ��� ����" << endl;
	return true;
}

/*
* @brief ���� ����
* @author �躴��
* @return ���� ���� ���� ����. true�� ����, false�� ����
*/
bool NetworkCore::StartServer()
{
	// ��Ŷ �Ŵ��� unique pointerȭ
	m_pPacketManager = make_unique<PacketManager>();

	// ��Ŷ �۽� �Լ������� ���ٽ����� ����
	m_pPacketManager->SendPacketFunc = [&](UINT32 clientIndex, UINT16 packetSize, char* pSendPacket)
		{
			// �۽�
			SendMsg(GetClientInfo(clientIndex), pSendPacket, packetSize);
		};

	// �۽��ϰ� ������ ����
	m_pPacketManager->Init();
	m_pPacketManager->Run();

	// �������� ���� Ȯ��
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	mMaxWorkerThread = sysInfo.dwNumberOfProcessors;

	// Ŭ���̾�Ʈ Ǯ �����
	CreateClient(MAX_CLIENT);

	// IOCP �ڵ� �ʱ�ȭ. ������� �� �������
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	// IOCP �Ϸ� ������ ����
	if (CreateWorkerThread() == false)
		return false;

	// Accept ������ ����
	if (CreateAcceptThread() == false)
		return false;

	cout << "���� ����" << endl;

	return true;
}

/*
* @brief ���� ����
* @author �躴��
* @return void
*/
void NetworkCore::EndServer()
{
	// Accept ������ ����
	mIsAcceptRun = false;
	if (mAcceptThread.joinable())
		mAcceptThread.join();

	// IOCP �ڵ� �ݱ�
	CloseHandle(mIOCPHandle);

	// IOCP �Ϸ� ������ ����
	mIsWorkerRun = false;
	for (auto& t : mIOWorkerThreads)
	{
		if (t.joinable())
			t.join();
	}

	// Listen ���� �ݱ�
	closesocket(mListenSocket);
}

/*
* @brief Ŭ���̾�Ʈ ����
* @author �躴��
* @param maxClientCount: ������ Ŭ���̾�Ʈ �� ����
* @return void
*/
void NetworkCore::CreateClient(const UINT32 maxClientCount)
{
	// Ŭ���̾�Ʈ vector ������
	int currentSize = mClientInfos.size();

	// ���� ������ + ���ϴ� �����ŭ �ø���
	int maxSize = currentSize + maxClientCount;
	for (UINT32 i = currentSize; i < maxSize; i++)
	{
		// Ŭ���̾�Ʈ �����Ҵ� �� �ε��� ����
		ClientInfo* client = new ClientInfo();
		client->SetIndex(i);

		mClientInfos.push_back(client);
	}

	// �ش� Ŭ���̾�Ʈ��ŭ ���� ����
	m_pPacketManager->AddSession(maxClientCount);
}

/*
* @brief �� Ŭ���̾�Ʈ ã��
* @author �躴��
* @return Ŭ���̾�Ʈ ���� ������
*/
ClientInfo* NetworkCore::GetEmptyClientInfo()
{
	// Ŭ���̾�Ʈ ���� �ϳ��� ����
	for (auto& client : mClientInfos)
	{
		// ������ ����ִٸ�. ��, ����Ǿ����� �ʴٸ�
		if (client->GetSocket() == INVALID_SOCKET)
			return client;
	}

	return nullptr;
}

/*
* @brief Ŭ���̾�Ʈ ���� ȣ��
* @author �躴��
* @param clientIndex Ŭ���̾�Ʈ �ε���
* @return Ŭ���̾�Ʈ ���� ������
*/
ClientInfo* NetworkCore::GetClientInfo(const UINT32 clientIndex)
{
	return mClientInfos[clientIndex];
}

/*
* @brief IOCP bind
* @author �躴��
* @param pClientInfo: Ŭ���̾�Ʈ ���� ������
* @return IOCP bind ���� ����. true�� ����, false�� ����
*/
bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	// Ŭ���̾�Ʈ�� ����� �������� �� ������ ������ IOCP �ڵ��� ���ؼ� �ޱ�. Key�� Ŭ���̾�Ʈ ������ ������ȭ
	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->GetSocket(), mIOCPHandle, (ULONG_PTR)pClientInfo, 0);
	if (hIOCP == NULL || mIOCPHandle != hIOCP)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

/*
* @brief WSARecv bind
* @author �躴��
* @param pClientInfo: Ŭ���̾�Ʈ ���� ������
* @return WSARecv bind ���� ����. true�� ����, false�� ����
*/
bool NetworkCore::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// ���� Overlapped Ȯ�� ����ü�� ���� �Է�
	pClientInfo->m_stRecvOverlapeedEx.m_wsaBuf.len = MAX_RECVBUF;
	pClientInfo->m_stRecvOverlapeedEx.m_wsaBuf.buf = pClientInfo->GetRecvBuffer();
	pClientInfo->m_stRecvOverlapeedEx.m_eOperation = IOOperation::RECV;

	// WSARecv
	int result = WSARecv(pClientInfo->GetSocket(),
		&(pClientInfo->m_stRecvOverlapeedEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (pClientInfo->m_stRecvOverlapeedEx),
		NULL);

	// WSARecv ����� ������ ���ų� IO_PENDING�� �ƴ϶��
	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSARecv() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

/*
* @brief WSASend bind
* @author �躴��
* @param pClientInfo: Ŭ���̾�Ʈ ���� ������
* @param pMsg: ��Ŷ ������
* @param nLen: ��Ŷ ����
* @return WSASend ���� ����. true�� ����, false�� ����.
*/
bool NetworkCore::SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen)
{
	DWORD dwSendNumBytes = 0;

	// �۽� ���� �ʱ�ȭ
	memcpy(pClientInfo->mSendBuf, pMsg, nLen);

	// �۽� Overlapped Ȯ�� ����ü ���� �Է�
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->GetSendBuffer();
	pClientInfo->m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

	// WSASend
	int result = WSASend(pClientInfo->GetSocket(),
		&(pClientInfo->m_stSendOverlappedEx.m_wsaBuf),
		1,
		&dwSendNumBytes,
		0,
		(LPWSAOVERLAPPED) & (pClientInfo->m_stSendOverlappedEx),
		NULL);

	// WSASend ����� ������ ���ų� IO_PENDING�� �ƴ϶��
	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSASend() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

/*
* @brief IOCP �۾� ������ ����
* @author �躴��
* @return ������ ���� ���� ����. true�� ����, false�� ����.
*/
bool NetworkCore::CreateWorkerThread()
{
	// ������ ����
	mIsWorkerRun = true;
	for (int i = 0; i < mMaxWorkerThread; i++)
	{
		mIOWorkerThreads.push_back(thread([this]()
			{
				WorkerThread();
			}));
	}

	cout << "WorkerThread ����" << endl;

	return true;
}

/*
* @brief IOCP �۾� ������
* @author �躴��
* @return void
*/
void NetworkCore::WorkerThread()
{
	ClientInfo* pClientInfo = nullptr;		// Ŭ���̾�Ʈ ������
	BOOL bSuccess = TRUE;					// GetQueuedCompletionStatus ���� ����
	DWORD dwIoSize = 0;						// IO�� ũ��
	LPOVERLAPPED lpOverlapped = nullptr;	// Overlapped ����ü ������

	// ���ѹݺ�
	while (mIsWorkerRun)
	{
		// IO �Ϸ� Queue���� �Ϸ�� �׸��� �� ������ ���
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo,
			&lpOverlapped, INFINITE);

		// �Ϸ�� �׸��� �ִµ�, ����� 0�̸鼭 Overlapped�� null�����͸�. ��, �������� ���� �����Ѵٸ�
		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == nullptr)
		{
			mIsWorkerRun = false;
			continue;
		}

		// Overlapped ����ü�� null �����͸�. ��, ����� �� ���� ���� ���ߴٸ�
		if (lpOverlapped == nullptr)
		{
			continue;
		}

		// �Ϸᰡ ���� �ʾҰų� �Ϸᰡ �Ǿ��µ� ����� 0�̶��. ��, Ŭ���̾�Ʈ�� ������ ������ ���
		if (bSuccess == FALSE || (bSuccess == TRUE && dwIoSize == 0))
		{
			cout << "Ŭ�� ���� ����! " << (int)pClientInfo->GetSocket() << endl;
			CloseSocket(pClientInfo);

			continue;
		}

		// Overlapped ����ü�� Ȯ�� ����ü�� ����ȯ
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		// �Ϸ� ������ Recv���ٸ�
		if (pOverlappedEx->m_eOperation == IOOperation::RECV)
		{
			// ��Ŷ �Ŵ������� ��Ŷ ���� ����
			m_pPacketManager->ReceivePacket(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuffer());

			// Recv ������ �ٽ� WSARecv ����. ���˴븦 ���������� �ٽ� ���˴븦 ������ ����⸦ ����~
			BindRecv(pClientInfo);
		}

		// �Ϸ� ������ Send���ٸ�
		else if (pOverlappedEx->m_eOperation == IOOperation::SEND)
		{
			cout << "[�۽�] bytes: " << dwIoSize << endl;
		}

		// ��� �ƴ� ��쿡�� ���ܻ�Ȳ�̴� �ٷ� ����
		else
		{
			cout << "socket(" << (int)pClientInfo->GetSocket() << ")���� ���ܻ�Ȳ" << endl;
			break;
		}
	}
}

/*
* @brief Accept ������ ����
* @author �躴��
* @return ������ ���� ���� ����. true�� ����, false�� ����
*/
bool NetworkCore::CreateAcceptThread()
{
	// ������ ����
	mIsAcceptRun = true;
	mAcceptThread = thread([this]()
		{
			AcceptThread();
		});

	cout << "AcceptThread ����" << endl;

	return true;
}

/*
* @brief Accept ������
* @author �躴��
* @return void
*/
void NetworkCore::AcceptThread()
{
	// Ŭ���̾�Ʈ �����ּ� ����
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	// ���ѹݺ�
	while (mIsAcceptRun)
	{
		// ����ִ� Ŭ���̾�Ʈ �� ã��
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == nullptr)
		{
			CreateClient(MAX_CLIENT);
			continue;
		}

		// accept
		SOCKET newSocket = accept(mListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (newSocket == INVALID_SOCKET)
			continue;

		// ���� ����
		pClientInfo->SetSocket(newSocket);

		// IOCP bind
		if (BindIoCompletionPort(pClientInfo) == false)
		{
			cout << "AcceptThread BindIoCompletionPort Error" << endl;
			return;
		}

		// WSARecv ����
		if (BindRecv(pClientInfo) == false)
		{
			cout << "AcceptThread BindRecv Error" << endl;
			return;
		}

		// Ŭ���̾�Ʈ ���� ���
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
		cout << "Ŭ�� ����: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->GetSocket() << ")" << endl;

		// Ŭ���̾�Ʈ ����
		m_pPacketManager->ConnectClient(pClientInfo->GetIndex());
	}
}

/*
* @brief ���� �ݱ�
* @author �躴��
* @param pClientInfo: Ŭ���̾�Ʈ ���� ������
* @param bIsForce: ��ó���� ���� ó�� ����
* @return void
*/
void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	// Ŭ���̾�Ʈ �ε��� ȣ��
	int clientIndex = pClientInfo->GetIndex();

	// ���� �ɼ� linger ����
	struct linger stLinger = { 0, 0 };

	// ���� ��ó���� ó���� �ִٸ� ��� ó�� �Ŀ� �����ϰԲ� ����
	if (bIsForce == true)
		stLinger.l_onoff = 1;

	// ���� ����
	shutdown(pClientInfo->GetSocket(), SD_BOTH);

	// ���� �ɼ� ����
	setsockopt(pClientInfo->GetSocket(), SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	// ���� �ݱ�
	closesocket(pClientInfo->GetSocket());

	// ���� ���� ������ ���� ������� �ʱ�ȭ
	pClientInfo->SetSocket(INVALID_SOCKET);

	// ��Ŷ �Ŵ����� Ŭ���̾�Ʈ �������� �˸���
	m_pPacketManager->DisconnectClient(clientIndex);
}
