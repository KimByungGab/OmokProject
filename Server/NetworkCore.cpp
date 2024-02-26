#include "NetworkCore.h"

// 소멸자
NetworkCore::~NetworkCore()
{
	WSACleanup();
}

/*
* @brief Listen 소켓 초기화
* @author 김병갑
* @return 초기화 성공 여부. true면 성공, false면 실패
*/
bool NetworkCore::InitSocket()
{
	// WSAData 초기화
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup() error: " << WSAGetLastError() << endl;
		return false;
	}

	// Listen 소켓 초기화
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (mListenSocket == INVALID_SOCKET)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "소켓 초기화 성공" << endl;
	return true;
}

/*
* @brief 소켓 bind, Listen
* @author 김병갑
* @return bind와 Listen 성공 여부. true면 성공, false면 실패
*/
bool NetworkCore::BindAndListen()
{
	// 소켓 정보 구조체
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 소켓 bind
	int result = ::bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (result != 0)
	{
		cout << "bind() error: " << WSAGetLastError() << endl;
		return false;
	}

	// 소켓 listen
	result = listen(mListenSocket, SOMAXCONN);
	if (result != 0)
	{
		cout << "listen() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "서버 등록 성공" << endl;
	return true;
}

/*
* @brief 서버 구동
* @author 김병갑
* @return 서버 구동 성공 여부. true면 성공, false면 실패
*/
bool NetworkCore::StartServer()
{
	// 패킷 매니저 unique pointer화
	m_pPacketManager = make_unique<PacketManager>();

	// 패킷 송신 함수변수에 람다식으로 정의
	m_pPacketManager->SendPacketFunc = [&](UINT32 clientIndex, UINT16 packetSize, char* pSendPacket)
		{
			// 송신
			SendMsg(GetClientInfo(clientIndex), pSendPacket, packetSize);
		};

	// 송신하고 쓰레드 구동
	m_pPacketManager->Init();
	m_pPacketManager->Run();

	// 쓰레드의 개수 확인
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	mMaxWorkerThread = sysInfo.dwNumberOfProcessors;

	// 클라이언트 풀 만들기
	CreateClient(MAX_CLIENT);

	// IOCP 핸들 초기화. 쓰레드는 내 쓰레드로
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	// IOCP 완료 쓰레드 생성
	if (CreateWorkerThread() == false)
		return false;

	// Accept 쓰레드 생성
	if (CreateAcceptThread() == false)
		return false;

	cout << "서버 시작" << endl;

	return true;
}

/*
* @brief 서버 종료
* @author 김병갑
* @return void
*/
void NetworkCore::EndServer()
{
	// Accept 쓰레드 종료
	mIsAcceptRun = false;
	if (mAcceptThread.joinable())
		mAcceptThread.join();

	// IOCP 핸들 닫기
	CloseHandle(mIOCPHandle);

	// IOCP 완료 쓰레드 종료
	mIsWorkerRun = false;
	for (auto& t : mIOWorkerThreads)
	{
		if (t.joinable())
			t.join();
	}

	// Listen 소켓 닫기
	closesocket(mListenSocket);
}

/*
* @brief 클라이언트 생성
* @author 김병갑
* @param maxClientCount: 생성할 클라이언트 방 개수
* @return void
*/
void NetworkCore::CreateClient(const UINT32 maxClientCount)
{
	// 클라이언트 vector 사이즈
	int currentSize = mClientInfos.size();

	// 원래 사이즈 + 원하는 사이즈만큼 늘리기
	int maxSize = currentSize + maxClientCount;
	for (UINT32 i = currentSize; i < maxSize; i++)
	{
		// 클라이언트 동적할당 후 인덱스 세팅
		ClientInfo* client = new ClientInfo();
		client->SetIndex(i);

		mClientInfos.push_back(client);
	}

	// 해당 클라이언트만큼 세션 연결
	m_pPacketManager->AddSession(maxClientCount);
}

/*
* @brief 빈 클라이언트 찾기
* @author 김병갑
* @return 클라이언트 정보 포인터
*/
ClientInfo* NetworkCore::GetEmptyClientInfo()
{
	// 클라이언트 정보 하나씩 보기
	for (auto& client : mClientInfos)
	{
		// 소켓이 비어있다면. 즉, 연결되어있지 않다면
		if (client->GetSocket() == INVALID_SOCKET)
			return client;
	}

	return nullptr;
}

/*
* @brief 클라이언트 정보 호출
* @author 김병갑
* @param clientIndex 클라이언트 인덱스
* @return 클라이언트 정보 포인터
*/
ClientInfo* NetworkCore::GetClientInfo(const UINT32 clientIndex)
{
	return mClientInfos[clientIndex];
}

/*
* @brief IOCP bind
* @author 김병갑
* @param pClientInfo: 클라이언트 정보 포인터
* @return IOCP bind 성공 여부. true면 성공, false면 실패
*/
bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	// 클라이언트에 연결된 소켓으로 온 정보는 지정된 IOCP 핸들을 통해서 받기. Key는 클라이언트 정보를 포인터화
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
* @author 김병갑
* @param pClientInfo: 클라이언트 정보 포인터
* @return WSARecv bind 성공 여부. true면 성공, false면 실패
*/
bool NetworkCore::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// 수신 Overlapped 확장 구조체에 정보 입력
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

	// WSARecv 결과가 에러가 났거나 IO_PENDING이 아니라면
	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSARecv() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

/*
* @brief WSASend bind
* @author 김병갑
* @param pClientInfo: 클라이언트 정보 포인터
* @param pMsg: 패킷 포인터
* @param nLen: 패킷 길이
* @return WSASend 성공 여부. true면 성공, false면 실패.
*/
bool NetworkCore::SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen)
{
	DWORD dwSendNumBytes = 0;

	// 송신 버퍼 초기화
	memcpy(pClientInfo->mSendBuf, pMsg, nLen);

	// 송신 Overlapped 확장 구조체 정보 입력
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

	// WSASend 결과가 에러가 났거나 IO_PENDING이 아니라면
	if (result == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSASend() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

/*
* @brief IOCP 작업 쓰레드 생성
* @author 김병갑
* @return 쓰레드 생성 성공 여부. true면 성공, false면 실패.
*/
bool NetworkCore::CreateWorkerThread()
{
	// 쓰레드 실행
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

/*
* @brief IOCP 작업 쓰레드
* @author 김병갑
* @return void
*/
void NetworkCore::WorkerThread()
{
	ClientInfo* pClientInfo = nullptr;		// 클라이언트 포인터
	BOOL bSuccess = TRUE;					// GetQueuedCompletionStatus 성공 여부
	DWORD dwIoSize = 0;						// IO한 크기
	LPOVERLAPPED lpOverlapped = nullptr;	// Overlapped 구조체 포인터

	// 무한반복
	while (mIsWorkerRun)
	{
		// IO 완료 Queue에서 완료된 항목이 올 때까지 대기
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo,
			&lpOverlapped, INFINITE);

		// 완료된 항목이 있는데, 사이즈가 0이면서 Overlapped가 null포인터면. 즉, 서버에서 직접 종료한다면
		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == nullptr)
		{
			mIsWorkerRun = false;
			continue;
		}

		// Overlapped 구조체가 null 포인터면. 즉, 제대로 된 값을 받지 못했다면
		if (lpOverlapped == nullptr)
		{
			continue;
		}

		// 완료가 되지 않았거나 완료가 되었는데 사이즈가 0이라면. 즉, 클라이언트가 강제로 종료한 경우
		if (bSuccess == FALSE || (bSuccess == TRUE && dwIoSize == 0))
		{
			cout << "클라 접속 종료! " << (int)pClientInfo->GetSocket() << endl;
			CloseSocket(pClientInfo);

			continue;
		}

		// Overlapped 구조체를 확장 구조체로 형변환
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		// 완료 정보가 Recv였다면
		if (pOverlappedEx->m_eOperation == IOOperation::RECV)
		{
			// 패킷 매니저에게 패킷 정보 수신
			m_pPacketManager->ReceivePacket(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuffer());

			// Recv 했으니 다시 WSARecv 실행. 낚싯대를 가져왔으면 다시 낚싯대를 던져야 물고기를 잡지~
			BindRecv(pClientInfo);
		}

		// 완료 정보가 Send였다면
		else if (pOverlappedEx->m_eOperation == IOOperation::SEND)
		{
			cout << "[송신] bytes: " << dwIoSize << endl;
		}

		// 모두 아닌 경우에는 예외상황이니 바로 종료
		else
		{
			cout << "socket(" << (int)pClientInfo->GetSocket() << ")에서 예외상황" << endl;
			break;
		}
	}
}

/*
* @brief Accept 쓰레드 생성
* @author 김병갑
* @return 쓰레드 생성 성공 여부. true면 성공, false면 실패
*/
bool NetworkCore::CreateAcceptThread()
{
	// 쓰레드 생성
	mIsAcceptRun = true;
	mAcceptThread = thread([this]()
		{
			AcceptThread();
		});

	cout << "AcceptThread 시작" << endl;

	return true;
}

/*
* @brief Accept 쓰레드
* @author 김병갑
* @return void
*/
void NetworkCore::AcceptThread()
{
	// 클라이언트 소켓주소 정보
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	// 무한반복
	while (mIsAcceptRun)
	{
		// 비어있는 클라이언트 방 찾기
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

		// 소켓 세팅
		pClientInfo->SetSocket(newSocket);

		// IOCP bind
		if (BindIoCompletionPort(pClientInfo) == false)
		{
			cout << "AcceptThread BindIoCompletionPort Error" << endl;
			return;
		}

		// WSARecv 실행
		if (BindRecv(pClientInfo) == false)
		{
			cout << "AcceptThread BindRecv Error" << endl;
			return;
		}

		// 클라이언트 정보 출력
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
		cout << "클라 접속: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->GetSocket() << ")" << endl;

		// 클라이언트 연결
		m_pPacketManager->ConnectClient(pClientInfo->GetIndex());
	}
}

/*
* @brief 소켓 닫기
* @author 김병갑
* @param pClientInfo: 클라이언트 정보 포인터
* @param bIsForce: 미처리된 정보 처리 여부
* @return void
*/
void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	// 클라이언트 인덱스 호출
	int clientIndex = pClientInfo->GetIndex();

	// 소켓 옵션 linger 세팅
	struct linger stLinger = { 0, 0 };

	// 만약 미처리된 처리가 있다면 모두 처리 후에 종료하게끔 세팅
	if (bIsForce == true)
		stLinger.l_onoff = 1;

	// 소켓 종료
	shutdown(pClientInfo->GetSocket(), SD_BOTH);

	// 소켓 옵션 세팅
	setsockopt(pClientInfo->GetSocket(), SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	// 소켓 닫기
	closesocket(pClientInfo->GetSocket());

	// 닫은 소켓 정보는 이제 원래대로 초기화
	pClientInfo->SetSocket(INVALID_SOCKET);

	// 패킷 매니저에 클라이언트 연결해제 알리기
	m_pPacketManager->DisconnectClient(clientIndex);
}
