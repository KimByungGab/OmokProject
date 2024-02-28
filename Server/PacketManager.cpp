#include "PacketManager.h"

/*
* @brief 패킷매니저 초기화
* @author 김병갑
* @return void
*/
void PacketManager::Init()
{
	// 함수 포인터 테이블 삽입
	mRecvFunctionTable[(int)PACKET_ID::LOGIN_REQ] = &PacketManager::LoginUser;
	mRecvFunctionTable[(int)PACKET_ID::LOGOUT_REQ] = &PacketManager::LogoutUser;
	mRecvFunctionTable[(int)PACKET_ID::GET_ROOMS_REQ] = &PacketManager::GetRooms;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_CREATE_REQ] = &PacketManager::CreateRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_ENTER_REQ] = &PacketManager::EnterRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_LEAVE_REQ] = &PacketManager::LeaveRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_CHAT_REQ] = &PacketManager::ChatRoom;
	mRecvFunctionTable[(int)PACKET_ID::GAME_READY_REQ] = &PacketManager::ReadyGame;
	mRecvFunctionTable[(int)PACKET_ID::GAME_CHOICE_PLACE_REQ] = &PacketManager::ChoicePlace;

	// 각 매니저 클래스 동적할당
	mRoomManager = new RoomManager();
	mSessionManager = new SessionManager();
	mGameManager = new GameManager();
	mDBConn = new DBConnection();
}

/*
* @brief 패킷 쓰레드 실행
* @author 김병갑
* @return void
*/
void PacketManager::Run()
{
	mIsProcessRun = true;
	mProcessThread = thread([this]()
		{
			ProcessPacket();
		});
}

/*
* @brief 패킷 쓰레드 종료
* @author 김병갑
* @return void
*/
void PacketManager::End()
{
	mIsProcessRun = false;
	if (mProcessThread.joinable())
		mProcessThread.join();
}

/*
* @brief 패킷 처리한 클라이언트 큐에 인덱스 삽입
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @return void
*/
void PacketManager::PushPacketClient(const UINT32 clientIndex)
{
	lock_guard<mutex> guard(mLock);
	mClientQueue.push(clientIndex);
}

/*
* @brief 수신된 패킷 처리
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param size: 패킷 사이즈
* @param pData: 패킷 포인터
* @return void
*/
void PacketManager::ReceivePacket(const UINT32 clientIndex, const UINT32 size, char* pData)
{
	// 클라이언트 인덱스로 세션 포인터 받기
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);

	// 세션에 할당되어있는 패킷 정보 삽입
	pSession->SetPacketData(size, pData);

	// 패킷 요청한 클라이언트 인덱스를 큐에 삽입
	PushPacketClient(clientIndex);
}

/*
* @brief 클라이언트 연결
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @return void
*/
void PacketManager::ConnectClient(UINT32 clientIndex)
{
	cout << "[ProcessUserConnect] clientIndex: " << clientIndex << endl;

	// 연결된 세션 초기화
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->Clear();
}

/*
* @brief 클라이언트 연결해제
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @return void
*/
void PacketManager::DisconnectClient(UINT32 clientIndex)
{
	cout << "[ProcessUserDisconnect] clientIndex: " << clientIndex << endl;

	// 클라이언트가 방 안에 있으면 퇴장
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	if (pSession->GetCurrentLocation() == Session::LOCATION::ROOM)
	{
		LeaveRoom(clientIndex, 0, nullptr);
	}

	// 사용했던 세션 클리어
	pSession->Clear();
}

/*
* @brief 세션 추가
* @author 김병갑
* @param addedCount: 추가되는 갯수
* @return void
*/
void PacketManager::AddSession(const int addedCount)
{
	mSessionManager->AddSession(addedCount);
}

/*
* @brief 패킷 데이터 호출
* @author 김병갑
* @return 패킷 정보
*/
PacketInfo PacketManager::GetPacketData()
{
	// 패킷을 받을 클라이언트 인덱스
	UINT32 clientIndex = 0;

	// 큐의 작업만 하기 위하여 락을 사용하기 위해 괄호 처리 (괄호 밖으로 나가면 락 끝)
	{
		lock_guard<mutex> guard(mLock);

		// 패킷 요청한 클라이언트 큐가 비어있으면 빈 패킷 반환
		if (mClientQueue.empty())
			return PacketInfo();

		// 클라이언트 정보 받기
		clientIndex = mClientQueue.front();
		mClientQueue.pop();
	}

	// 클라이언트의 세션 정보 호출
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);

	// 세션의 패킷 정보 받기
	PacketInfo packetData = pSession->GetPacket();
	packetData.ClientIndex = clientIndex;

	return packetData;
}

/*
* @brief 패킷 쓰레드 작업함수
* @author 김병갑
* @return void
*/
void PacketManager::ProcessPacket()
{
	// 쓰레드를 계속 실행시켜야된다면 무한반복
	while (mIsProcessRun)
	{
		// 패킷 데이터 받기
		auto packetData = GetPacketData();

		// 패킷 ID가 시스템 종료 이상이라면 (모든 정보요청)
		if (packetData.PacketID > (UINT16)PACKET_ID::SYS_END)
		{
			// 패킷 ID와 연결된 함수 실행
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketID,
				packetData.DataSize, packetData.pDataPtr);
		}
		else
		{
			// 그 외라면 sleep
			this_thread::sleep_for(1ms);
		}
	}
}

/*
* @brief 받은 패킷 실행
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetID: 패킷 ID
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetID, const UINT16 packetSize, char* pPacket)
{
	// 패킷 ID와 연결된 함수 포인터 찾기
	auto iter = mRecvFunctionTable.find(packetID);
	if (iter != mRecvFunctionTable.end())
	{
		// 함수 포인터에 연결된 함수 실행
		(this->*(iter->second))(clientIndex, packetSize, pPacket);
	}
}

/*
* @brief 유저 로그인
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::LoginUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	bool LoginResult = false;	// 로그인 결과 여부
	int userIdx = 0;			// 유저 인덱스
	
	// 패킷 정보를 변환
	auto pLoginReqPacket = reinterpret_cast<LOGIN_REQ_PACKET*>(pPacket);

	// SQL 세팅
	vector<string> column;
	column.push_back("IDX");
	string whereStr = "id = '" + string(pLoginReqPacket->ID) + "' AND pw = '" + string(pLoginReqPacket->PW) + "'";

	mDBConn->SetTable("test_table");
	auto result = mDBConn->SelectData(column, whereStr);

	// 사이즈가 0이 아니라면 (SQL문 에러가 아닌데 0이라면 원하는 정보가 없다는 뜻)
	if (result.size() != 0)
	{
		// IDX 컬럼 iterator 찾기
		auto iter = result[0].find("IDX");

		// 원하는 정보가 있다면
		if (iter != result[0].end())
		{
			LoginResult = true;					// 로그인 성공
			userIdx = stoi(iter->second);		// 받은 value를 string->int로 형변환
		}
	}

	// 로그인 응답 패킷
	LOGIN_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::LOGIN_RES;
	packet.PacketLength = sizeof(LOGIN_RES_PACKET);

	// 로그인 성공했다면
	if (LoginResult)
	{
		// 로그인 여부 성공
		packet.LoginResult = true;

		// 세션에 id, idx값 세팅 후 Lobby로 이동
		Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
		pSession->SetId(string(pLoginReqPacket->ID));
		pSession->SetIdx(userIdx);
		pSession->EnterLobby();
	}

	// 실패했다면
	else
	{
		// 로그인 여부 실패
		packet.LoginResult = false;
	}

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief 유저 로그아웃
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::LogoutUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// 로그아웃 패킷 구조체
	LOGOUT_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::LOGOUT_RES;
	packet.PacketLength = sizeof(LOGOUT_RES_PACKET);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief 모든 방 정보 호출
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::GetRooms(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// 패킷 정보 형변환
	auto pGetRoomsReqPacket = reinterpret_cast<GET_ROOMS_REQ_PACKET*>(pPacket);
	
	// 패킷 리스트 호출
	auto rooms = mRoomManager->GetRooms();

	// 모든 방 정보 호출 응답 패킷
	GET_ROOMS_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::GET_ROOMS_RES;
	packet.PacketLength = sizeof(GET_ROOMS_RES_PACKET);
	packet.totalRoomCount = 0;

	// 패킷의 배열 인덱스용 변수
	int i = 0;

	// 리스트 전부 순회
	for (auto pRoom : rooms)
	{
		// 방 이름
		wstring roomName = pRoom->roomName;
		roomName += L"\0";

		// 방 인덱스 삽입
		packet.roomInfos[i].roomIndex = pRoom->roomNumber;

		// 방 이름 삽입
		memset(packet.roomInfos[i].roomName, 0, sizeof(packet.roomInfos[i].roomName));
		memcpy(packet.roomInfos[i].roomName, pRoom->roomName, roomName.size() * 2);

		// 방 이름 Length 삽입
		packet.roomInfos[i].roomNameSize = roomName.size();

		// 방 현재 인원수 삽입
		packet.roomInfos[i].currentUserCount = pRoom->roomMembers.size();

		// 방 총 가능 인원수 삽입
		packet.roomInfos[i].totalUserCount = mRoomManager->mMaxRoomMemberCount;

		i++;	// 패킷의 배열 인덱스 증가
		packet.totalRoomCount++;	// 총 방 개수 증가
	}

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief 방 생성
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::CreateRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomCreateReqPacket = reinterpret_cast<ROOM_CREATE_REQ_PACKET*>(pPacket);

	// 방 이름 
	WCHAR* roomName = pRoomCreateReqPacket->roomName;

	// 방 생성
	COMMON_RESULT_DTO result = mRoomManager->CreateRoom(roomName);

	// 방 생성에 실패했다면
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}

	// 방 정보
	lobby_room_info roomInfo = *(reinterpret_cast<lobby_room_info*>(result.pObject));

	// 방 삽입으로 인한 갱신 패킷
	INSERT_ROOM_LIST_PACKET insertRoomListPacket;
	insertRoomListPacket.PacketID = (UINT16)PACKET_ID::INSERT_ROOM;
	insertRoomListPacket.PacketLength = sizeof(INSERT_ROOM_LIST_PACKET);
	insertRoomListPacket.index = mRoomManager->GetListIndex(roomInfo.roomIndex);
	insertRoomListPacket.roomInfo = roomInfo;

	// 로비에 있는 모든 유저들에게 삽입 내용 전송
	vector<Session*> currentLobbySessions = mSessionManager->GetLobbySessions();
	for (auto pSession : currentLobbySessions)
	{
		SendPacketFunc(pSession->GetConnIdx(), sizeof(insertRoomListPacket), reinterpret_cast<char*>(&insertRoomListPacket));
	}

	// 방 생성 응답
	ROOM_CREATE_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_CREATE_RES;
	packet.PacketLength = sizeof(ROOM_CREATE_RES_PACKET);
	packet.roomIndex = roomInfo.roomIndex;

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));

	// 방에 해당되는 게임 생성
	mGameManager->CreateGame(roomInfo.roomIndex);
}

/*
* @brief 방 입장
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::EnterRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomEnterReqPacket = reinterpret_cast<ROOM_ENTER_REQ_PACKET*>(pPacket);

	// 입장할 방 인덱스
	int roomIndex = pRoomEnterReqPacket->roomIndex;

	// 방 입장
	COMMON_RESULT_DTO result = mRoomManager->EnterRoom(clientIndex, roomIndex);

	// 방 입장을 실패했다면
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}
	
	// 입장할 방 정보를 세션에 삽입
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->EnterRoom(roomIndex);

	// 방 입장 응답
	ROOM_ENTER_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_ENTER_RES;
	packet.PacketLength = sizeof(ROOM_ENTER_RES_PACKET);
	
	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));

	// 채팅 응답
	ROOM_CHAT_RES_PACKET roomChatResPacket;
	roomChatResPacket.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
	roomChatResPacket.PacketLength = sizeof(ROOM_CHAT_RES_PACKET);

	// 시스템에서 입장 채팅 작성
	string id = pSession->GetId();
	wstring systemChat = wstring().assign(id.begin(), id.end());
	systemChat += L"님께서 " + to_wstring(roomIndex) + L"번 방에 들어왔습니다.\0";
	memcpy(roomChatResPacket.chatData, systemChat.c_str(), systemChat.size() * 2);

	roomChatResPacket.chatDataSize = systemChat.size();

	// 방에 있는 모든 유저에게 시스템 채팅 정보 전송
	list<int> currentRoomMemberList = mRoomManager->GetMembersInRoom(roomIndex);
	for (int sessionIndex : currentRoomMemberList)
	{
		SendPacketFunc(sessionIndex, sizeof(roomChatResPacket), reinterpret_cast<char*>(&roomChatResPacket));
	}

	// 방 리스트의 실제 인덱스 호출
	int roomListIndex = mRoomManager->GetListIndex(roomIndex);
	if (roomListIndex == -1)
		return;

	// 방 유저수 갱신 패킷
	UPDATE_ROOM_USER_PACKET updateRoomUserPacket;
	updateRoomUserPacket.PacketID = (UINT16)PACKET_ID::UPDATE_ROOM;
	updateRoomUserPacket.PacketLength = sizeof(UPDATE_ROOM_USER_PACKET);
	updateRoomUserPacket.index = roomListIndex;
	updateRoomUserPacket.currentUserCount = currentRoomMemberList.size();

	// 로비에 있는 모든 유저에게 방 정보 전송
	auto currentLobbyUserList = mSessionManager->GetLobbySessions();
	for (Session* session : currentLobbyUserList)
	{
		SendPacketFunc(session->GetConnIdx(), sizeof(updateRoomUserPacket), reinterpret_cast<char*>(&updateRoomUserPacket));
	}
}

/*
* @brief 방 퇴장
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::LeaveRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// 세션을 얻어 방 인덱스 호출
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();

	// 방과 연결되어있는 게임의 포인터 호출
	Game* pGame = mGameManager->GetGame(roomIndex);

	// 게임준비를 한 세션 리스트 얻기
	list<int> players = pGame->GetReadySession();

	// 본인의 세션이 있는지 확인
	auto playersIter = find_if(players.begin(), players.end(), [&](const int& element)
		{
			return clientIndex == element;
		});

	// 만약 내가 준비를 한 상태였다면
	if (playersIter != players.end())
	{
		// 만약 게임을 하고있는 중이었다면
		if (mRoomManager->GetPlayStatus(roomIndex))
		{
			// 플레이어의 세션 map 호출
			map<int, USHORT> playersMap = pGame->GetPlayerSession();

			// 상대방의 정보 호출. 상대방을 승리하게 만들기 위함
			auto opponentMap = find_if(playersMap.begin(), playersMap.end(), [&](const pair<int, USHORT>& element)
				{
					return clientIndex != element.first;
				});

			// 상대방이 있다면
			if (opponentMap != playersMap.end())
			{
				// 게임오버 패킷으로 상대방을 승리시키기
				GAME_OVER_PACKET gameOverPacket;
				gameOverPacket.PacketID = (UINT16)PACKET_ID::GAME_OVER;
				gameOverPacket.PacketLength = sizeof(GAME_OVER_PACKET);
				gameOverPacket.winnerSide = opponentMap->second;

				// 본인은 어차피 방을 나가는 것이기 때문에 상대방에게만 전송
				SendPacketFunc(opponentMap->first, sizeof(gameOverPacket), reinterpret_cast<char*>(&gameOverPacket));
			}

			// 플레이어 리스트 리셋
			pGame->ResetPlayers();

			// 상태가 '게임중이 아님'으로 변경
			mRoomManager->SetPlayStatus(roomIndex, false);
		}

		// 게임을 하는 중이 아니었다면. 즉, 준비만 한 상태였다면
		else
		{
			// 준비해제
			COMMON_RESULT_DTO playerResult = pGame->Ready(clientIndex);

			// 준비해제가 잘 되었다면
			if (playerResult.code == ERROR_CODE::NOTHING)
			{
				// 초기화를 하기 위한 더미
				PlayerInfo dummy;
				dummy.idx = 0;
				memset(dummy.id, 0, sizeof(dummy.id));

				// 게임 준비 응답
				GAME_READY_RES_PACKET readyPacket;
				readyPacket.PacketID = (UINT16)PACKET_ID::GAME_READY_RES;
				readyPacket.PacketLength = sizeof(GAME_READY_RES_PACKET);
				readyPacket.playerInfos[0] = dummy;
				readyPacket.playerInfos[1] = dummy;

				// 게임 준비 응답 패킷의 플레이어 정보 배열 인덱스
				int i = 0;

				// 준비한 플레이어 리스트 호출
				list<int> players = *(reinterpret_cast<list<int>*>(playerResult.pObject));

				// 준비한 플레이어에게 플레이어 정보 삽입
				for (int playerSession : players)
				{
					Session* pPlayerSession = mSessionManager->GetUserByConnIdx(playerSession);
					string sessionId = pPlayerSession->GetId();
					wstring sessionIdWstr = wstring().assign(sessionId.begin(), sessionId.end());

					PlayerInfo info;
					info.idx = pPlayerSession->GetIdx();
					memcpy(info.id, sessionIdWstr.c_str(), sessionIdWstr.size() * 2);
					info.idSize = sessionIdWstr.size();

					readyPacket.playerInfos[i] = info;

					i++;
				}

				// 방 안의 모든 유저에게 준비상태를 전송
				list<int> roomMembers = mRoomManager->GetMembersInRoom(roomIndex);
				for (int memberSession : roomMembers)
				{
					SendPacketFunc(memberSession, sizeof(readyPacket), reinterpret_cast<char*>(&readyPacket));
				}
			}
		}
	}

	// 방 퇴장
	COMMON_RESULT_DTO result = mRoomManager->LeaveRoom(clientIndex, roomIndex);

	// 퇴장 실패했다면
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}

	// 퇴장을 했는데 방에 아무도 없을 경우
	if (strcmp(reinterpret_cast<char*>(result.pObject), mRoomManager->mEmptyString) == 0)
	{
		// 방 제거
		result = mRoomManager->DeleteRoom(roomIndex);

		// 제거가 잘 되었다면
		if (result.code != ERROR_CODE::NOTHING)
		{
			ErrorSend(clientIndex, result.code);

			return;
		}

		// 방과 연결되어있는 게임 제거
		mGameManager->RemoveGame(roomIndex);

		// 방이 삭제되었으니 로비에 알리기
		REMOVE_ROOM_LIST_PACKET removeRoomListPacket;
		removeRoomListPacket.PacketID = (UINT16)PACKET_ID::REMOVE_ROOM;
		removeRoomListPacket.PacketLength = sizeof(REMOVE_ROOM_LIST_PACKET);
		removeRoomListPacket.index = reinterpret_cast<int>(result.pObject);

		vector<Session*> currentLobbySessions = mSessionManager->GetLobbySessions();
		for (auto pCurrentSession : currentLobbySessions)
		{
			SendPacketFunc(pCurrentSession->GetConnIdx(), sizeof(removeRoomListPacket), reinterpret_cast<char*>(&removeRoomListPacket));
		}
	}

	// 방에 다른 사람이 있는 경우
	else
	{
		// 시스템 채팅으로 방 인원 전부에게 퇴장 알리기
		ROOM_CHAT_RES_PACKET roomChatResPacket;
		roomChatResPacket.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
		roomChatResPacket.PacketLength = sizeof(roomChatResPacket);
		
		string idStr = pSession->GetId();
		wstring idWstr = wstring().assign(idStr.begin(), idStr.end());
		wstring systemChat = idWstr + L"님꼐서 퇴장하셨습니다.\0";

		memcpy(roomChatResPacket.chatData, systemChat.c_str(), systemChat.size() * 2);
		roomChatResPacket.chatDataSize = systemChat.size();

		list<int> members = mRoomManager->GetMembersInRoom(roomIndex);
		for (int sessionIndex : members)
		{
			SendPacketFunc(sessionIndex, sizeof(roomChatResPacket), reinterpret_cast<char*>(&roomChatResPacket));
		}

		// 참여한 사람의 숫자가 줄어들었으니 로비에게 알리기
		UPDATE_ROOM_USER_PACKET updateRoomUserPacket;
		updateRoomUserPacket.PacketID = (UINT16)PACKET_ID::UPDATE_ROOM;
		updateRoomUserPacket.PacketLength = sizeof(UPDATE_ROOM_USER_PACKET);
		updateRoomUserPacket.index = mRoomManager->GetListIndex(roomIndex);
		updateRoomUserPacket.currentUserCount = members.size();

		auto currentLobbyUserList = mSessionManager->GetLobbySessions();
		for (Session* session : currentLobbyUserList)
		{
			SendPacketFunc(session->GetConnIdx(), sizeof(updateRoomUserPacket), reinterpret_cast<char*>(&updateRoomUserPacket));
		}
	}

	// 방에서 나간 사람은 로비로 이동
	pSession->EnterLobby();

	// 방 퇴장 응답
	ROOM_LEAVE_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_LEAVE_RES;
	packet.PacketLength = sizeof(ROOM_LEAVE_RES_PACKET);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief 채팅
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::ChatRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomChatReqPacket = reinterpret_cast<ROOM_CHAT_REQ_PACKET*>(pPacket);

	// 채팅 내용
	wstring chatData = pRoomChatReqPacket->chatData;

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	string myIdStr = pSession->GetId();
	wstring myIdWstr = wstring().assign(myIdStr.begin(), myIdStr.end());

	// 전송할 채팅 내용
	wstring sendedChat = L"[" + myIdWstr + L"]: " + chatData + L"\0";

	// 채팅 응답
	ROOM_CHAT_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
	packet.PacketLength = sizeof(ROOM_CHAT_RES_PACKET);
	memcpy(packet.chatData, sendedChat.c_str(), sendedChat.size() * 2);

	packet.chatDataSize = sendedChat.size();

	// 방의 모든 인원에게 전송
	list<int> roomMembers = mRoomManager->GetMembersInRoom(pSession->GetCurrentRoom());
	for (int member : roomMembers)
	{
		SendPacketFunc(member, sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

/*
* @brief 게임 준비
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::ReadyGame(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// 세션을 이용해서 게임 정보 가져오기
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();
	Game* pGame = mGameManager->GetGame(roomIndex);

	// 게임 준비
	COMMON_RESULT_DTO playerResult = pGame->Ready(clientIndex);

	// 게임준비를 실패했다면
	if (playerResult.code != ERROR_CODE::NOTHING)
	{
		SYS_USER_ERROR_PACKET errorPacket;
		errorPacket.PacketID = (UINT16)PACKET_ID::SYS_ERROR;
		errorPacket.PacketLength = sizeof(SYS_USER_ERROR_PACKET);
		errorPacket.errorCode = static_cast<unsigned short>(playerResult.code);

		SendPacketFunc(clientIndex, sizeof(errorPacket), reinterpret_cast<char*>(&errorPacket));
		return;
	}

	// 초기화를 위한 더미
	PlayerInfo dummy;
	dummy.idx = 0;
	memset(dummy.id, 0, sizeof(dummy.id));

	// 게임 준비 응답 패킷
	GAME_READY_RES_PACKET readyPacket;
	readyPacket.PacketID = (UINT16)PACKET_ID::GAME_READY_RES;
	readyPacket.PacketLength = sizeof(GAME_READY_RES_PACKET);
	readyPacket.playerInfos[0] = dummy;
	readyPacket.playerInfos[1] = dummy;

	// 패킷의 플레이어 정보 배열 인덱스
	int i = 0;

	// 준비한 플레이어들 호출
	list<int> players = *(reinterpret_cast<list<int>*>(playerResult.pObject));

	// 플레이어 정보 입력
	for (int playerSession : players)
	{
		Session* pPlayerSession = mSessionManager->GetUserByConnIdx(playerSession);
		string sessionId = pPlayerSession->GetId();
		wstring sessionIdWstr = wstring().assign(sessionId.begin(), sessionId.end());

		PlayerInfo info;
		info.idx = pPlayerSession->GetIdx();
		memcpy(info.id, sessionIdWstr.c_str(), sessionIdWstr.size() * 2);
		info.idSize = sessionIdWstr.size();

		readyPacket.playerInfos[i] = info;
		
		i++;
	}

	// 해당 정보를 방의 모든 인원에게 전송
	list<int> roomMembers = mRoomManager->GetMembersInRoom(roomIndex);
	for (int memberSession : roomMembers)
	{
		SendPacketFunc(memberSession, sizeof(readyPacket), reinterpret_cast<char*>(&readyPacket));
	}

	// 게임준비한 인원이 게임에 필요한 사람만큼 없는 경우는 끝
	if (players.size() != pGame->GetMaxPlayerCount())
		return;

	// 게임 내용 클리어 후 시작
	pGame->Clear();
	pGame->Start();

	// 게임 중
	mRoomManager->SetPlayStatus(roomIndex, true);

	// 현재 정해진 플레이어 세션 map 호출
	map<int, USHORT> playerSession = pGame->GetPlayerSession();

	// 게임 시작 패킷
	GAME_START_PACKET gameStartPacket;
	gameStartPacket.PacketID = (UINT16)PACKET_ID::GAME_START;
	gameStartPacket.PacketLength = sizeof(GAME_START_PACKET);

	// 플레이어들에게 전송
	for (int player : players)
	{
		// map 내의 해당 플레이어와 관련된 정보 찾기
		auto iter = playerSession.find(player);

		// 찾지 못했다면 다음으로 이동
		if (iter == playerSession.end())
			continue;

		// 본인의 색깔 정보 입력
		gameStartPacket.omokSide = iter->second;

		SendPacketFunc(player, sizeof(gameStartPacket), reinterpret_cast<char*>(&gameStartPacket));
	}
}

/*
* @brief 좌표에 돌 두기
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param packetSize: 패킷 사이즈
* @param pPacket: 패킷 포인터
* @return void
*/
void PacketManager::ChoicePlace(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pGameTurnReqPacket = reinterpret_cast<GAME_TURN_REQ_PACKET*>(pPacket);

	// 해당 턴의 좌표 정보 얻기
	// 레퍼런스로 한 이유: 만드는 것 자체가 아깝다고 생각했기 때문. 정보를 건드리지도 않을것이기 때문에 원본을 가져와도 상관없기 때문이다.
	GameTurnPlaceInfo& refGameTurnPlaceInfo = pGameTurnReqPacket->placeInfo;

	// 세션을 이용해서 게임 정보를 얻기
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();
	Game* pGame = mGameManager->GetGame(roomIndex);

	// 좌표에 돌 두기
	COMMON_RESULT_DTO result = pGame->ChoicePlace(clientIndex, refGameTurnPlaceInfo.horizontalNum, refGameTurnPlaceInfo.verticalNum);

	// 돌을 두는 데 실패했다면
	if (result.code != ERROR_CODE::NOTHING)
		return;

	// 해당 게임 턴을 전달받기. (필요하지 않다고 생각한다. ref에 있는 값을 그대로 쓰는거와 마찬가지이니)
	GameTurnPlaceInfo* pResultPlaceInfo = reinterpret_cast<GameTurnPlaceInfo*>(result.pObject);

	// 게임 턴 결과 패킷
	GAME_TURN_RESULT_PACKET gameTurnResultPacket;
	gameTurnResultPacket.PacketID = (UINT16)PACKET_ID::GAME_CHOICE_PLACE_RESULT;
	gameTurnResultPacket.PacketLength = sizeof(GAME_TURN_RESULT_PACKET);
	gameTurnResultPacket.result = *pResultPlaceInfo;

	// 플레이어들에게 전달
	map<int, USHORT> playersMap = pGame->GetPlayerSession();
	for (auto iter = playersMap.begin(); iter != playersMap.end(); ++iter)
	{
		SendPacketFunc(iter->first, sizeof(gameTurnResultPacket), reinterpret_cast<char*>(&gameTurnResultPacket));
	}

	// 승리 여부 판단
	bool isGameOver = pGame->ConfirmWin(refGameTurnPlaceInfo.omokSide, refGameTurnPlaceInfo.horizontalNum, refGameTurnPlaceInfo.verticalNum);

	// 아직 승리하지 못했다면
	if (isGameOver == false)
		return;

	// 승리했으니 게임오버
	GAME_OVER_PACKET gameOverPacket;
	gameOverPacket.PacketID = (UINT16)PACKET_ID::GAME_OVER;
	gameOverPacket.PacketLength = sizeof(gameOverPacket);
	gameOverPacket.winnerSide = refGameTurnPlaceInfo.omokSide;

	for (auto iter = playersMap.begin(); iter != playersMap.end(); ++iter)
	{
		SendPacketFunc(iter->first, sizeof(gameOverPacket), reinterpret_cast<char*>(&gameOverPacket));
	}

	// 게임 플레이어 리스트 리셋
	pGame->ResetPlayers();

	// 게임 중이 아님
	mRoomManager->SetPlayStatus(roomIndex, false);
}

/*
* @brief 에러 전송
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param code: 에러 코드
* @return void
*/
void PacketManager::ErrorSend(UINT32 clientIndex, ERROR_CODE code)
{
	// 에러 패킷
	SYS_USER_ERROR_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::SYS_ERROR;
	packet.PacketLength = sizeof(SYS_USER_ERROR_PACKET);
	packet.errorCode = static_cast<unsigned short>(code);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}
