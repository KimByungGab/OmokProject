#include "PacketManager.h"

void PacketManager::Init(const UINT32 maxClient)
{
	mRecvFunctionTable[(int)PACKET_ID::LOGIN_REQ] = &PacketManager::LoginUser;
	mRecvFunctionTable[(int)PACKET_ID::LOGOUT_REQ] = &PacketManager::LogoutUser;
	mRecvFunctionTable[(int)PACKET_ID::GET_ROOMS_REQ] = &PacketManager::GetRooms;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_CREATE_REQ] = &PacketManager::CreateRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_ENTER_REQ] = &PacketManager::EnterRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_LEAVE_REQ] = &PacketManager::LeaveRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_CHAT_REQ] = &PacketManager::ChatRoom;
	mRecvFunctionTable[(int)PACKET_ID::GAME_READY_REQ] = &PacketManager::ReadyGame;
	mRecvFunctionTable[(int)PACKET_ID::GAME_CHOICE_PLACE_REQ] = &PacketManager::ChoicePlace;

	mRoomManager = new RoomManager();
	mSessionManager = new SessionManager();
	mGameManager = new GameManager();
	mDBConn = new DBConnection();
}

void PacketManager::Run()
{
	mIsProcessRun = true;
	mProcessThread = thread([this]()
		{
			ProcessPacket();
		});
}

void PacketManager::End()
{
	mIsProcessRun = false;
	if (mProcessThread.joinable())
		mProcessThread.join();
}

void PacketManager::PushPacketClient(const UINT32 clientIndex)
{
	lock_guard<mutex> guard(mLock);
	mClientQueue.push(clientIndex);
}

void PacketManager::ReceivePacket(const UINT32 clientIndex, const UINT32 size, char* pData)
{
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->SetPacketData(size, pData);

	PushPacketClient(clientIndex);
}

void PacketManager::ConnectClient(UINT32 clientIndex)
{
	cout << "[ProcessUserConnect] clientIndex: " << clientIndex << endl;

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->Clear();
}

void PacketManager::DisconnectClient(UINT32 clientIndex)
{
	cout << "[ProcessUserDisconnect] clientIndex: " << clientIndex << endl;

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	if (pSession->GetCurrentLocation() == Session::LOCATION::ROOM)
	{
		LeaveRoom(clientIndex, 0, nullptr);
	}

	pSession->Clear();
}

void PacketManager::AddSession(const int addedCount)
{
	mSessionManager->AddSession(addedCount);
}

PacketInfo PacketManager::GetPacketData()
{
	UINT32 clientIndex = 0;

	{
		lock_guard<mutex> guard(mLock);
		if (mClientQueue.empty())
			return PacketInfo();

		clientIndex = mClientQueue.front();
		mClientQueue.pop();
	}

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);

	PacketInfo packetData = pSession->GetPacket();
	packetData.ClientIndex = clientIndex;

	return packetData;
}

void PacketManager::ProcessPacket()
{
	while (mIsProcessRun)
	{
		auto packetData = GetPacketData();

		if (packetData.PacketID > (UINT16)PACKET_ID::SYS_END)
		{
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketID,
				packetData.DataSize, packetData.pDataPtr);
		}
		else
		{
			this_thread::sleep_for(1ms);
		}
	}
}

void PacketManager::ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetID, const UINT16 packetSize, char* pPacket)
{
	auto iter = mRecvFunctionTable.find(packetID);
	if (iter != mRecvFunctionTable.end())
	{
		(this->*(iter->second))(clientIndex, packetSize, pPacket);
	}
}

void PacketManager::LoginUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	bool LoginResult = false;
	int userIdx = 0;
	
	auto pLoginReqPacket = reinterpret_cast<LOGIN_REQ_PACKET*>(pPacket);

	vector<string> column;
	column.push_back("IDX");
	string whereStr = "id = '" + string(pLoginReqPacket->ID) + "' AND pw = '" + string(pLoginReqPacket->PW) + "'";

	mDBConn->SetTable("test_table");
	auto result = mDBConn->SelectData(column, whereStr);
	if (result.size() != 0)
	{
		auto iter = result[0].find("IDX");
		if (iter != result[0].end())
		{
			LoginResult = true;
			userIdx = stoi(iter->second);
		}
	}

	LOGIN_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::LOGIN_RES;
	packet.PacketLength = sizeof(LOGIN_RES_PACKET);

	if (LoginResult)
	{
		packet.LoginResult = true;

		Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
		pSession->SetId(string(pLoginReqPacket->ID));
		pSession->SetIdx(userIdx);
		pSession->EnterLobby();
	}
	else
	{
		packet.LoginResult = false;
	}

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

void PacketManager::LogoutUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	LOGOUT_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::LOGOUT_RES;
	packet.PacketLength = sizeof(LOGOUT_RES_PACKET);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

void PacketManager::GetRooms(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pGetRoomsReqPacket = reinterpret_cast<GET_ROOMS_REQ_PACKET*>(pPacket);
	
	auto rooms = mRoomManager->GetRooms();

	GET_ROOMS_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::GET_ROOMS_RES;
	packet.PacketLength = sizeof(GET_ROOMS_RES_PACKET);
	packet.totalRoomCount = 0;

	int i = 0;
	for (auto iter : rooms)
	{
		wstring roomName = iter->roomName;
		roomName += L"\0";

		packet.roomInfos[i].roomIndex = iter->roomNumber;
		memset(packet.roomInfos[i].roomName, 0, sizeof(packet.roomInfos[i].roomName));
		memcpy(packet.roomInfos[i].roomName, iter->roomName, roomName.size() * 2);
		packet.roomInfos[i].roomNameSize = roomName.size();
		packet.roomInfos[i].currentUserCount = iter->roomMembers.size();
		packet.roomInfos[i].totalUserCount = mRoomManager->mMaxRoomMemberCount;
		i++;
		packet.totalRoomCount++;
	}

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

void PacketManager::CreateRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomCreateReqPacket = reinterpret_cast<ROOM_CREATE_REQ_PACKET*>(pPacket);
	WCHAR* roomName = pRoomCreateReqPacket->roomName;

	COMMON_RESULT_DTO result = mRoomManager->CreateRoom(roomName);
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}

	lobby_room_info roomInfo = *(reinterpret_cast<lobby_room_info*>(result.pObject));

	INSERT_ROOM_LIST_PACKET insertRoomListPacket;
	insertRoomListPacket.PacketID = (UINT16)PACKET_ID::INSERT_ROOM;
	insertRoomListPacket.PacketLength = sizeof(INSERT_ROOM_LIST_PACKET);
	insertRoomListPacket.index = mRoomManager->GetListIndex(roomInfo.roomIndex);
	insertRoomListPacket.roomInfo = roomInfo;

	vector<Session*> currentLobbySessions = mSessionManager->GetLobbySessions();
	for (auto pSession : currentLobbySessions)
	{
		SendPacketFunc(pSession->GetConnIdx(), sizeof(insertRoomListPacket), reinterpret_cast<char*>(&insertRoomListPacket));
	}

	ROOM_CREATE_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_CREATE_RES;
	packet.PacketLength = sizeof(ROOM_CREATE_RES_PACKET);
	packet.roomIndex = roomInfo.roomIndex;

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));

	mGameManager->CreateGame(roomInfo.roomIndex);
}

void PacketManager::EnterRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomEnterReqPacket = reinterpret_cast<ROOM_ENTER_REQ_PACKET*>(pPacket);

	int roomIndex = pRoomEnterReqPacket->roomIndex;

	COMMON_RESULT_DTO result = mRoomManager->EnterRoom(clientIndex, roomIndex);

	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}
	
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->EnterRoom(roomIndex);

	ROOM_ENTER_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_ENTER_RES;
	packet.PacketLength = sizeof(ROOM_ENTER_RES_PACKET);
	
	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));

	ROOM_CHAT_RES_PACKET roomChatResPacket;
	roomChatResPacket.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
	roomChatResPacket.PacketLength = sizeof(ROOM_CHAT_RES_PACKET);

	string id = pSession->GetId();
	wstring systemChat = wstring().assign(id.begin(), id.end());
	systemChat += L"´Ô²²¼­ " + to_wstring(roomIndex) + L"¹ø ¹æ¿¡ µé¾î¿Ô½À´Ï´Ù.\0";
	memcpy(roomChatResPacket.chatData, systemChat.c_str(), systemChat.size() * 2);

	roomChatResPacket.chatDataSize = systemChat.size();

	list<int> currentRoomMemberList = mRoomManager->GetMembersInRoom(roomIndex);
	for (int sessionIndex : currentRoomMemberList)
	{
		SendPacketFunc(sessionIndex, sizeof(roomChatResPacket), reinterpret_cast<char*>(&roomChatResPacket));
	}

	int roomListIndex = mRoomManager->GetListIndex(roomIndex);
	if (roomListIndex == -1)
		return;

	UPDATE_ROOM_USER_PACKET updateRoomUserPacket;
	updateRoomUserPacket.PacketID = (UINT16)PACKET_ID::UPDATE_ROOM;
	updateRoomUserPacket.PacketLength = sizeof(UPDATE_ROOM_USER_PACKET);
	updateRoomUserPacket.index = roomListIndex;
	updateRoomUserPacket.currentUserCount = currentRoomMemberList.size();

	auto currentLobbyUserList = mSessionManager->GetLobbySessions();
	for (Session* session : currentLobbyUserList)
	{
		SendPacketFunc(session->GetConnIdx(), sizeof(updateRoomUserPacket), reinterpret_cast<char*>(&updateRoomUserPacket));
	}
}

void PacketManager::LeaveRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();

	Game* pGame = mGameManager->GetGame(roomIndex);
	list<int> players = pGame->GetReadySession();
	auto playersIter = find_if(players.begin(), players.end(), [&](const int& element)
		{
			return clientIndex == element;
		});
	if (playersIter != players.end())
	{
		if (mRoomManager->GetPlayStatus(roomIndex))
		{
			map<int, USHORT> playersMap = pGame->GetPlayerSession();

			auto opponentMap = find_if(playersMap.begin(), playersMap.end(), [&](const pair<int, USHORT>& element)
				{
					return clientIndex != element.first;
				});
			if (opponentMap != playersMap.end())
			{
				GAME_OVER_PACKET gameOverPacket;
				gameOverPacket.PacketID = (UINT16)PACKET_ID::GAME_OVER;
				gameOverPacket.PacketLength = sizeof(GAME_OVER_PACKET);
				gameOverPacket.winnerSide = opponentMap->second;

				SendPacketFunc(opponentMap->first, sizeof(gameOverPacket), reinterpret_cast<char*>(&gameOverPacket));
			}

			pGame->ResetPlayers();

			mRoomManager->SetPlayStatus(roomIndex, false);
		}
		else
		{
			COMMON_RESULT_DTO playerResult = pGame->Ready(clientIndex);
			if (playerResult.code == ERROR_CODE::NOTHING)
			{
				PlayerInfo dummy;
				dummy.idx = 0;
				memset(dummy.id, 0, sizeof(dummy.id));

				GAME_READY_RES_PACKET readyPacket;
				readyPacket.PacketID = (UINT16)PACKET_ID::GAME_READY_RES;
				readyPacket.PacketLength = sizeof(GAME_READY_RES_PACKET);
				readyPacket.playerInfos[0] = dummy;
				readyPacket.playerInfos[1] = dummy;

				int i = 0;
				list<int> players = *(reinterpret_cast<list<int>*>(playerResult.pObject));
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

				list<int> roomMembers = mRoomManager->GetMembersInRoom(roomIndex);
				for (int memberSession : roomMembers)
				{
					SendPacketFunc(memberSession, sizeof(readyPacket), reinterpret_cast<char*>(&readyPacket));
				}
			}
		}
	}

	COMMON_RESULT_DTO result = mRoomManager->LeaveRoom(clientIndex, roomIndex);
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}

	if (strcmp(result.pObject, mRoomManager->mEmptyString) == 0)
	{
		result = mRoomManager->DeleteRoom(roomIndex);
		if (result.code != ERROR_CODE::NOTHING)
		{
			ErrorSend(clientIndex, result.code);

			return;
		}

		mGameManager->RemoveGame(roomIndex);

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
	else
	{
		ROOM_CHAT_RES_PACKET roomChatResPacket;
		roomChatResPacket.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
		roomChatResPacket.PacketLength = sizeof(roomChatResPacket);
		
		string idStr = pSession->GetId();
		wstring idWstr = wstring().assign(idStr.begin(), idStr.end());
		wstring systemChat = idWstr + L"´Ô²¾¼­ ÅðÀåÇÏ¼Ì½À´Ï´Ù.\0";

		memcpy(roomChatResPacket.chatData, systemChat.c_str(), systemChat.size() * 2);
		roomChatResPacket.chatDataSize = systemChat.size();

		list<int> members = mRoomManager->GetMembersInRoom(roomIndex);
		for (int sessionIndex : members)
		{
			SendPacketFunc(sessionIndex, sizeof(roomChatResPacket), reinterpret_cast<char*>(&roomChatResPacket));
		}

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

	pSession->EnterLobby();

	ROOM_LEAVE_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_LEAVE_RES;
	packet.PacketLength = sizeof(ROOM_LEAVE_RES_PACKET);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

void PacketManager::ChatRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomChatReqPacket = reinterpret_cast<ROOM_CHAT_REQ_PACKET*>(pPacket);

	wstring chatData = pRoomChatReqPacket->chatData;

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	string myIdStr = pSession->GetId();
	wstring myIdWstr = wstring().assign(myIdStr.begin(), myIdStr.end());

	wstring sendedChat = L"[" + myIdWstr + L"]: " + chatData + L"\0";

	ROOM_CHAT_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
	packet.PacketLength = sizeof(ROOM_CHAT_RES_PACKET);
	memcpy(packet.chatData, sendedChat.c_str(), sendedChat.size() * 2);

	packet.chatDataSize = sendedChat.size();

	list<int> roomMembers = mRoomManager->GetMembersInRoom(pSession->GetCurrentRoom());
	for (int member : roomMembers)
	{
		SendPacketFunc(member, sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

void PacketManager::ReadyGame(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();
	Game* pGame = mGameManager->GetGame(roomIndex);

	COMMON_RESULT_DTO playerResult = pGame->Ready(clientIndex);
	if (playerResult.code != ERROR_CODE::NOTHING)
	{
		SYS_USER_ERROR_PACKET errorPacket;
		errorPacket.PacketID = (UINT16)PACKET_ID::SYS_ERROR;
		errorPacket.PacketLength = sizeof(SYS_USER_ERROR_PACKET);
		errorPacket.errorCode = static_cast<unsigned short>(playerResult.code);

		SendPacketFunc(clientIndex, sizeof(errorPacket), reinterpret_cast<char*>(&errorPacket));
		return;
	}

	PlayerInfo dummy;
	dummy.idx = 0;
	memset(dummy.id, 0, sizeof(dummy.id));

	GAME_READY_RES_PACKET readyPacket;
	readyPacket.PacketID = (UINT16)PACKET_ID::GAME_READY_RES;
	readyPacket.PacketLength = sizeof(GAME_READY_RES_PACKET);
	readyPacket.playerInfos[0] = dummy;
	readyPacket.playerInfos[1] = dummy;

	int i = 0;
	list<int> players = *(reinterpret_cast<list<int>*>(playerResult.pObject));
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

	list<int> roomMembers = mRoomManager->GetMembersInRoom(roomIndex);
	for (int memberSession : roomMembers)
	{
		SendPacketFunc(memberSession, sizeof(readyPacket), reinterpret_cast<char*>(&readyPacket));
	}

	if (players.size() != mGameManager->GetMaxPlayerCount())
		return;

	pGame->Clear();
	pGame->Start();

	mRoomManager->SetPlayStatus(roomIndex, true);

	map<int, USHORT> playerSession = pGame->GetPlayerSession();

	GAME_START_PACKET gameStartPacket;
	gameStartPacket.PacketID = (UINT16)PACKET_ID::GAME_START;
	gameStartPacket.PacketLength = sizeof(GAME_START_PACKET);

	for (int player : players)
	{
		auto iter = playerSession.find(player);
		if (iter == playerSession.end())
			continue;

		gameStartPacket.omokSide = iter->second;

		SendPacketFunc(player, sizeof(gameStartPacket), reinterpret_cast<char*>(&gameStartPacket));
	}
}

void PacketManager::ChoicePlace(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pGameTurnReqPacket = reinterpret_cast<GAME_TURN_REQ_PACKET*>(pPacket);
	GameTurnPlaceInfo& refGameTurnPlaceInfo = pGameTurnReqPacket->placeInfo;

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();

	Game* pGame = mGameManager->GetGame(roomIndex);
	COMMON_RESULT_DTO result = pGame->ChoicePlace(clientIndex, refGameTurnPlaceInfo.horizontalNum, refGameTurnPlaceInfo.verticalNum);

	if (result.code != ERROR_CODE::NOTHING)
		return;

	GameTurnPlaceInfo* pResultPlaceInfo = reinterpret_cast<GameTurnPlaceInfo*>(result.pObject);

	GAME_TURN_RESULT_PACKET gameTurnResultPacket;
	gameTurnResultPacket.PacketID = (UINT16)PACKET_ID::GAME_CHOICE_PLACE_RESULT;
	gameTurnResultPacket.PacketLength = sizeof(GAME_TURN_RESULT_PACKET);
	gameTurnResultPacket.result = *pResultPlaceInfo;

	map<int, USHORT> playersMap = pGame->GetPlayerSession();

	for (auto iter = playersMap.begin(); iter != playersMap.end(); ++iter)
	{
		SendPacketFunc(iter->first, sizeof(gameTurnResultPacket), reinterpret_cast<char*>(&gameTurnResultPacket));
	}

	bool isGameOver = pGame->ConfirmWin(refGameTurnPlaceInfo.omokSide, refGameTurnPlaceInfo.horizontalNum, refGameTurnPlaceInfo.verticalNum);
	if (isGameOver == false)
		return;

	GAME_OVER_PACKET gameOverPacket;
	gameOverPacket.PacketID = (UINT16)PACKET_ID::GAME_OVER;
	gameOverPacket.PacketLength = sizeof(gameOverPacket);
	gameOverPacket.winnerSide = refGameTurnPlaceInfo.omokSide;

	for (auto iter = playersMap.begin(); iter != playersMap.end(); ++iter)
	{
		SendPacketFunc(iter->first, sizeof(gameOverPacket), reinterpret_cast<char*>(&gameOverPacket));
	}

	pGame->ResetPlayers();

	mRoomManager->SetPlayStatus(roomIndex, false);
}

void PacketManager::ErrorSend(UINT32 clientIndex, ERROR_CODE code)
{
	SYS_USER_ERROR_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::SYS_ERROR;
	packet.PacketLength = sizeof(SYS_USER_ERROR_PACKET);
	packet.errorCode = static_cast<unsigned short>(code);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}
