#include "PacketManager.h"

/*
* @brief ��Ŷ�Ŵ��� �ʱ�ȭ
* @author �躴��
* @return void
*/
void PacketManager::Init()
{
	// �Լ� ������ ���̺� ����
	mRecvFunctionTable[(int)PACKET_ID::LOGIN_REQ] = &PacketManager::LoginUser;
	mRecvFunctionTable[(int)PACKET_ID::LOGOUT_REQ] = &PacketManager::LogoutUser;
	mRecvFunctionTable[(int)PACKET_ID::GET_ROOMS_REQ] = &PacketManager::GetRooms;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_CREATE_REQ] = &PacketManager::CreateRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_ENTER_REQ] = &PacketManager::EnterRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_LEAVE_REQ] = &PacketManager::LeaveRoom;
	mRecvFunctionTable[(int)PACKET_ID::ROOM_CHAT_REQ] = &PacketManager::ChatRoom;
	mRecvFunctionTable[(int)PACKET_ID::GAME_READY_REQ] = &PacketManager::ReadyGame;
	mRecvFunctionTable[(int)PACKET_ID::GAME_CHOICE_PLACE_REQ] = &PacketManager::ChoicePlace;

	// �� �Ŵ��� Ŭ���� �����Ҵ�
	mRoomManager = new RoomManager();
	mSessionManager = new SessionManager();
	mGameManager = new GameManager();
	mDBConn = new DBConnection();
}

/*
* @brief ��Ŷ ������ ����
* @author �躴��
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
* @brief ��Ŷ ������ ����
* @author �躴��
* @return void
*/
void PacketManager::End()
{
	mIsProcessRun = false;
	if (mProcessThread.joinable())
		mProcessThread.join();
}

/*
* @brief ��Ŷ ó���� Ŭ���̾�Ʈ ť�� �ε��� ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @return void
*/
void PacketManager::PushPacketClient(const UINT32 clientIndex)
{
	lock_guard<mutex> guard(mLock);
	mClientQueue.push(clientIndex);
}

/*
* @brief ���ŵ� ��Ŷ ó��
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param size: ��Ŷ ������
* @param pData: ��Ŷ ������
* @return void
*/
void PacketManager::ReceivePacket(const UINT32 clientIndex, const UINT32 size, char* pData)
{
	// Ŭ���̾�Ʈ �ε����� ���� ������ �ޱ�
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);

	// ���ǿ� �Ҵ�Ǿ��ִ� ��Ŷ ���� ����
	pSession->SetPacketData(size, pData);

	// ��Ŷ ��û�� Ŭ���̾�Ʈ �ε����� ť�� ����
	PushPacketClient(clientIndex);
}

/*
* @brief Ŭ���̾�Ʈ ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @return void
*/
void PacketManager::ConnectClient(UINT32 clientIndex)
{
	cout << "[ProcessUserConnect] clientIndex: " << clientIndex << endl;

	// ����� ���� �ʱ�ȭ
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->Clear();
}

/*
* @brief Ŭ���̾�Ʈ ��������
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @return void
*/
void PacketManager::DisconnectClient(UINT32 clientIndex)
{
	cout << "[ProcessUserDisconnect] clientIndex: " << clientIndex << endl;

	// Ŭ���̾�Ʈ�� �� �ȿ� ������ ����
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	if (pSession->GetCurrentLocation() == Session::LOCATION::ROOM)
	{
		LeaveRoom(clientIndex, 0, nullptr);
	}

	// ����ߴ� ���� Ŭ����
	pSession->Clear();
}

/*
* @brief ���� �߰�
* @author �躴��
* @param addedCount: �߰��Ǵ� ����
* @return void
*/
void PacketManager::AddSession(const int addedCount)
{
	mSessionManager->AddSession(addedCount);
}

/*
* @brief ��Ŷ ������ ȣ��
* @author �躴��
* @return ��Ŷ ����
*/
PacketInfo PacketManager::GetPacketData()
{
	// ��Ŷ�� ���� Ŭ���̾�Ʈ �ε���
	UINT32 clientIndex = 0;

	// ť�� �۾��� �ϱ� ���Ͽ� ���� ����ϱ� ���� ��ȣ ó�� (��ȣ ������ ������ �� ��)
	{
		lock_guard<mutex> guard(mLock);

		// ��Ŷ ��û�� Ŭ���̾�Ʈ ť�� ��������� �� ��Ŷ ��ȯ
		if (mClientQueue.empty())
			return PacketInfo();

		// Ŭ���̾�Ʈ ���� �ޱ�
		clientIndex = mClientQueue.front();
		mClientQueue.pop();
	}

	// Ŭ���̾�Ʈ�� ���� ���� ȣ��
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);

	// ������ ��Ŷ ���� �ޱ�
	PacketInfo packetData = pSession->GetPacket();
	packetData.ClientIndex = clientIndex;

	return packetData;
}

/*
* @brief ��Ŷ ������ �۾��Լ�
* @author �躴��
* @return void
*/
void PacketManager::ProcessPacket()
{
	// �����带 ��� ������Ѿߵȴٸ� ���ѹݺ�
	while (mIsProcessRun)
	{
		// ��Ŷ ������ �ޱ�
		auto packetData = GetPacketData();

		// ��Ŷ ID�� �ý��� ���� �̻��̶�� (��� ������û)
		if (packetData.PacketID > (UINT16)PACKET_ID::SYS_END)
		{
			// ��Ŷ ID�� ����� �Լ� ����
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketID,
				packetData.DataSize, packetData.pDataPtr);
		}
		else
		{
			// �� �ܶ�� sleep
			this_thread::sleep_for(1ms);
		}
	}
}

/*
* @brief ���� ��Ŷ ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetID: ��Ŷ ID
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetID, const UINT16 packetSize, char* pPacket)
{
	// ��Ŷ ID�� ����� �Լ� ������ ã��
	auto iter = mRecvFunctionTable.find(packetID);
	if (iter != mRecvFunctionTable.end())
	{
		// �Լ� �����Ϳ� ����� �Լ� ����
		(this->*(iter->second))(clientIndex, packetSize, pPacket);
	}
}

/*
* @brief ���� �α���
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::LoginUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	bool LoginResult = false;	// �α��� ��� ����
	int userIdx = 0;			// ���� �ε���
	
	// ��Ŷ ������ ��ȯ
	auto pLoginReqPacket = reinterpret_cast<LOGIN_REQ_PACKET*>(pPacket);

	// SQL ����
	vector<string> column;
	column.push_back("IDX");
	string whereStr = "id = '" + string(pLoginReqPacket->ID) + "' AND pw = '" + string(pLoginReqPacket->PW) + "'";

	mDBConn->SetTable("test_table");
	auto result = mDBConn->SelectData(column, whereStr);

	// ����� 0�� �ƴ϶�� (SQL�� ������ �ƴѵ� 0�̶�� ���ϴ� ������ ���ٴ� ��)
	if (result.size() != 0)
	{
		// IDX �÷� iterator ã��
		auto iter = result[0].find("IDX");

		// ���ϴ� ������ �ִٸ�
		if (iter != result[0].end())
		{
			LoginResult = true;					// �α��� ����
			userIdx = stoi(iter->second);		// ���� value�� string->int�� ����ȯ
		}
	}

	// �α��� ���� ��Ŷ
	LOGIN_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::LOGIN_RES;
	packet.PacketLength = sizeof(LOGIN_RES_PACKET);

	// �α��� �����ߴٸ�
	if (LoginResult)
	{
		// �α��� ���� ����
		packet.LoginResult = true;

		// ���ǿ� id, idx�� ���� �� Lobby�� �̵�
		Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
		pSession->SetId(string(pLoginReqPacket->ID));
		pSession->SetIdx(userIdx);
		pSession->EnterLobby();
	}

	// �����ߴٸ�
	else
	{
		// �α��� ���� ����
		packet.LoginResult = false;
	}

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief ���� �α׾ƿ�
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::LogoutUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// �α׾ƿ� ��Ŷ ����ü
	LOGOUT_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::LOGOUT_RES;
	packet.PacketLength = sizeof(LOGOUT_RES_PACKET);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief ��� �� ���� ȣ��
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::GetRooms(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// ��Ŷ ���� ����ȯ
	auto pGetRoomsReqPacket = reinterpret_cast<GET_ROOMS_REQ_PACKET*>(pPacket);
	
	// ��Ŷ ����Ʈ ȣ��
	auto rooms = mRoomManager->GetRooms();

	// ��� �� ���� ȣ�� ���� ��Ŷ
	GET_ROOMS_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::GET_ROOMS_RES;
	packet.PacketLength = sizeof(GET_ROOMS_RES_PACKET);
	packet.totalRoomCount = 0;

	// ��Ŷ�� �迭 �ε����� ����
	int i = 0;

	// ����Ʈ ���� ��ȸ
	for (auto pRoom : rooms)
	{
		// �� �̸�
		wstring roomName = pRoom->roomName;
		roomName += L"\0";

		// �� �ε��� ����
		packet.roomInfos[i].roomIndex = pRoom->roomNumber;

		// �� �̸� ����
		memset(packet.roomInfos[i].roomName, 0, sizeof(packet.roomInfos[i].roomName));
		memcpy(packet.roomInfos[i].roomName, pRoom->roomName, roomName.size() * 2);

		// �� �̸� Length ����
		packet.roomInfos[i].roomNameSize = roomName.size();

		// �� ���� �ο��� ����
		packet.roomInfos[i].currentUserCount = pRoom->roomMembers.size();

		// �� �� ���� �ο��� ����
		packet.roomInfos[i].totalUserCount = mRoomManager->mMaxRoomMemberCount;

		i++;	// ��Ŷ�� �迭 �ε��� ����
		packet.totalRoomCount++;	// �� �� ���� ����
	}

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief �� ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::CreateRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomCreateReqPacket = reinterpret_cast<ROOM_CREATE_REQ_PACKET*>(pPacket);

	// �� �̸� 
	WCHAR* roomName = pRoomCreateReqPacket->roomName;

	// �� ����
	COMMON_RESULT_DTO result = mRoomManager->CreateRoom(roomName);

	// �� ������ �����ߴٸ�
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}

	// �� ����
	lobby_room_info roomInfo = *(reinterpret_cast<lobby_room_info*>(result.pObject));

	// �� �������� ���� ���� ��Ŷ
	INSERT_ROOM_LIST_PACKET insertRoomListPacket;
	insertRoomListPacket.PacketID = (UINT16)PACKET_ID::INSERT_ROOM;
	insertRoomListPacket.PacketLength = sizeof(INSERT_ROOM_LIST_PACKET);
	insertRoomListPacket.index = mRoomManager->GetListIndex(roomInfo.roomIndex);
	insertRoomListPacket.roomInfo = roomInfo;

	// �κ� �ִ� ��� �����鿡�� ���� ���� ����
	vector<Session*> currentLobbySessions = mSessionManager->GetLobbySessions();
	for (auto pSession : currentLobbySessions)
	{
		SendPacketFunc(pSession->GetConnIdx(), sizeof(insertRoomListPacket), reinterpret_cast<char*>(&insertRoomListPacket));
	}

	// �� ���� ����
	ROOM_CREATE_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_CREATE_RES;
	packet.PacketLength = sizeof(ROOM_CREATE_RES_PACKET);
	packet.roomIndex = roomInfo.roomIndex;

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));

	// �濡 �ش�Ǵ� ���� ����
	mGameManager->CreateGame(roomInfo.roomIndex);
}

/*
* @brief �� ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::EnterRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomEnterReqPacket = reinterpret_cast<ROOM_ENTER_REQ_PACKET*>(pPacket);

	// ������ �� �ε���
	int roomIndex = pRoomEnterReqPacket->roomIndex;

	// �� ����
	COMMON_RESULT_DTO result = mRoomManager->EnterRoom(clientIndex, roomIndex);

	// �� ������ �����ߴٸ�
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}
	
	// ������ �� ������ ���ǿ� ����
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	pSession->EnterRoom(roomIndex);

	// �� ���� ����
	ROOM_ENTER_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_ENTER_RES;
	packet.PacketLength = sizeof(ROOM_ENTER_RES_PACKET);
	
	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));

	// ä�� ����
	ROOM_CHAT_RES_PACKET roomChatResPacket;
	roomChatResPacket.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
	roomChatResPacket.PacketLength = sizeof(ROOM_CHAT_RES_PACKET);

	// �ý��ۿ��� ���� ä�� �ۼ�
	string id = pSession->GetId();
	wstring systemChat = wstring().assign(id.begin(), id.end());
	systemChat += L"�Բ��� " + to_wstring(roomIndex) + L"�� �濡 ���Խ��ϴ�.\0";
	memcpy(roomChatResPacket.chatData, systemChat.c_str(), systemChat.size() * 2);

	roomChatResPacket.chatDataSize = systemChat.size();

	// �濡 �ִ� ��� �������� �ý��� ä�� ���� ����
	list<int> currentRoomMemberList = mRoomManager->GetMembersInRoom(roomIndex);
	for (int sessionIndex : currentRoomMemberList)
	{
		SendPacketFunc(sessionIndex, sizeof(roomChatResPacket), reinterpret_cast<char*>(&roomChatResPacket));
	}

	// �� ����Ʈ�� ���� �ε��� ȣ��
	int roomListIndex = mRoomManager->GetListIndex(roomIndex);
	if (roomListIndex == -1)
		return;

	// �� ������ ���� ��Ŷ
	UPDATE_ROOM_USER_PACKET updateRoomUserPacket;
	updateRoomUserPacket.PacketID = (UINT16)PACKET_ID::UPDATE_ROOM;
	updateRoomUserPacket.PacketLength = sizeof(UPDATE_ROOM_USER_PACKET);
	updateRoomUserPacket.index = roomListIndex;
	updateRoomUserPacket.currentUserCount = currentRoomMemberList.size();

	// �κ� �ִ� ��� �������� �� ���� ����
	auto currentLobbyUserList = mSessionManager->GetLobbySessions();
	for (Session* session : currentLobbyUserList)
	{
		SendPacketFunc(session->GetConnIdx(), sizeof(updateRoomUserPacket), reinterpret_cast<char*>(&updateRoomUserPacket));
	}
}

/*
* @brief �� ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::LeaveRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// ������ ��� �� �ε��� ȣ��
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();

	// ��� ����Ǿ��ִ� ������ ������ ȣ��
	Game* pGame = mGameManager->GetGame(roomIndex);

	// �����غ� �� ���� ����Ʈ ���
	list<int> players = pGame->GetReadySession();

	// ������ ������ �ִ��� Ȯ��
	auto playersIter = find_if(players.begin(), players.end(), [&](const int& element)
		{
			return clientIndex == element;
		});

	// ���� ���� �غ� �� ���¿��ٸ�
	if (playersIter != players.end())
	{
		// ���� ������ �ϰ��ִ� ���̾��ٸ�
		if (mRoomManager->GetPlayStatus(roomIndex))
		{
			// �÷��̾��� ���� map ȣ��
			map<int, USHORT> playersMap = pGame->GetPlayerSession();

			// ������ ���� ȣ��. ������ �¸��ϰ� ����� ����
			auto opponentMap = find_if(playersMap.begin(), playersMap.end(), [&](const pair<int, USHORT>& element)
				{
					return clientIndex != element.first;
				});

			// ������ �ִٸ�
			if (opponentMap != playersMap.end())
			{
				// ���ӿ��� ��Ŷ���� ������ �¸���Ű��
				GAME_OVER_PACKET gameOverPacket;
				gameOverPacket.PacketID = (UINT16)PACKET_ID::GAME_OVER;
				gameOverPacket.PacketLength = sizeof(GAME_OVER_PACKET);
				gameOverPacket.winnerSide = opponentMap->second;

				// ������ ������ ���� ������ ���̱� ������ ���濡�Ը� ����
				SendPacketFunc(opponentMap->first, sizeof(gameOverPacket), reinterpret_cast<char*>(&gameOverPacket));
			}

			// �÷��̾� ����Ʈ ����
			pGame->ResetPlayers();

			// ���°� '�������� �ƴ�'���� ����
			mRoomManager->SetPlayStatus(roomIndex, false);
		}

		// ������ �ϴ� ���� �ƴϾ��ٸ�. ��, �غ� �� ���¿��ٸ�
		else
		{
			// �غ�����
			COMMON_RESULT_DTO playerResult = pGame->Ready(clientIndex);

			// �غ������� �� �Ǿ��ٸ�
			if (playerResult.code == ERROR_CODE::NOTHING)
			{
				// �ʱ�ȭ�� �ϱ� ���� ����
				PlayerInfo dummy;
				dummy.idx = 0;
				memset(dummy.id, 0, sizeof(dummy.id));

				// ���� �غ� ����
				GAME_READY_RES_PACKET readyPacket;
				readyPacket.PacketID = (UINT16)PACKET_ID::GAME_READY_RES;
				readyPacket.PacketLength = sizeof(GAME_READY_RES_PACKET);
				readyPacket.playerInfos[0] = dummy;
				readyPacket.playerInfos[1] = dummy;

				// ���� �غ� ���� ��Ŷ�� �÷��̾� ���� �迭 �ε���
				int i = 0;

				// �غ��� �÷��̾� ����Ʈ ȣ��
				list<int> players = *(reinterpret_cast<list<int>*>(playerResult.pObject));

				// �غ��� �÷��̾�� �÷��̾� ���� ����
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

				// �� ���� ��� �������� �غ���¸� ����
				list<int> roomMembers = mRoomManager->GetMembersInRoom(roomIndex);
				for (int memberSession : roomMembers)
				{
					SendPacketFunc(memberSession, sizeof(readyPacket), reinterpret_cast<char*>(&readyPacket));
				}
			}
		}
	}

	// �� ����
	COMMON_RESULT_DTO result = mRoomManager->LeaveRoom(clientIndex, roomIndex);

	// ���� �����ߴٸ�
	if (result.code != ERROR_CODE::NOTHING)
	{
		ErrorSend(clientIndex, result.code);

		return;
	}

	// ������ �ߴµ� �濡 �ƹ��� ���� ���
	if (strcmp(reinterpret_cast<char*>(result.pObject), mRoomManager->mEmptyString) == 0)
	{
		// �� ����
		result = mRoomManager->DeleteRoom(roomIndex);

		// ���Ű� �� �Ǿ��ٸ�
		if (result.code != ERROR_CODE::NOTHING)
		{
			ErrorSend(clientIndex, result.code);

			return;
		}

		// ��� ����Ǿ��ִ� ���� ����
		mGameManager->RemoveGame(roomIndex);

		// ���� �����Ǿ����� �κ� �˸���
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

	// �濡 �ٸ� ����� �ִ� ���
	else
	{
		// �ý��� ä������ �� �ο� ���ο��� ���� �˸���
		ROOM_CHAT_RES_PACKET roomChatResPacket;
		roomChatResPacket.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
		roomChatResPacket.PacketLength = sizeof(roomChatResPacket);
		
		string idStr = pSession->GetId();
		wstring idWstr = wstring().assign(idStr.begin(), idStr.end());
		wstring systemChat = idWstr + L"�Բ��� �����ϼ̽��ϴ�.\0";

		memcpy(roomChatResPacket.chatData, systemChat.c_str(), systemChat.size() * 2);
		roomChatResPacket.chatDataSize = systemChat.size();

		list<int> members = mRoomManager->GetMembersInRoom(roomIndex);
		for (int sessionIndex : members)
		{
			SendPacketFunc(sessionIndex, sizeof(roomChatResPacket), reinterpret_cast<char*>(&roomChatResPacket));
		}

		// ������ ����� ���ڰ� �پ������� �κ񿡰� �˸���
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

	// �濡�� ���� ����� �κ�� �̵�
	pSession->EnterLobby();

	// �� ���� ����
	ROOM_LEAVE_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_LEAVE_RES;
	packet.PacketLength = sizeof(ROOM_LEAVE_RES_PACKET);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}

/*
* @brief ä��
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::ChatRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pRoomChatReqPacket = reinterpret_cast<ROOM_CHAT_REQ_PACKET*>(pPacket);

	// ä�� ����
	wstring chatData = pRoomChatReqPacket->chatData;

	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	string myIdStr = pSession->GetId();
	wstring myIdWstr = wstring().assign(myIdStr.begin(), myIdStr.end());

	// ������ ä�� ����
	wstring sendedChat = L"[" + myIdWstr + L"]: " + chatData + L"\0";

	// ä�� ����
	ROOM_CHAT_RES_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::ROOM_CHAT_RES;
	packet.PacketLength = sizeof(ROOM_CHAT_RES_PACKET);
	memcpy(packet.chatData, sendedChat.c_str(), sendedChat.size() * 2);

	packet.chatDataSize = sendedChat.size();

	// ���� ��� �ο����� ����
	list<int> roomMembers = mRoomManager->GetMembersInRoom(pSession->GetCurrentRoom());
	for (int member : roomMembers)
	{
		SendPacketFunc(member, sizeof(packet), reinterpret_cast<char*>(&packet));
	}
}

/*
* @brief ���� �غ�
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::ReadyGame(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	// ������ �̿��ؼ� ���� ���� ��������
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();
	Game* pGame = mGameManager->GetGame(roomIndex);

	// ���� �غ�
	COMMON_RESULT_DTO playerResult = pGame->Ready(clientIndex);

	// �����غ� �����ߴٸ�
	if (playerResult.code != ERROR_CODE::NOTHING)
	{
		SYS_USER_ERROR_PACKET errorPacket;
		errorPacket.PacketID = (UINT16)PACKET_ID::SYS_ERROR;
		errorPacket.PacketLength = sizeof(SYS_USER_ERROR_PACKET);
		errorPacket.errorCode = static_cast<unsigned short>(playerResult.code);

		SendPacketFunc(clientIndex, sizeof(errorPacket), reinterpret_cast<char*>(&errorPacket));
		return;
	}

	// �ʱ�ȭ�� ���� ����
	PlayerInfo dummy;
	dummy.idx = 0;
	memset(dummy.id, 0, sizeof(dummy.id));

	// ���� �غ� ���� ��Ŷ
	GAME_READY_RES_PACKET readyPacket;
	readyPacket.PacketID = (UINT16)PACKET_ID::GAME_READY_RES;
	readyPacket.PacketLength = sizeof(GAME_READY_RES_PACKET);
	readyPacket.playerInfos[0] = dummy;
	readyPacket.playerInfos[1] = dummy;

	// ��Ŷ�� �÷��̾� ���� �迭 �ε���
	int i = 0;

	// �غ��� �÷��̾�� ȣ��
	list<int> players = *(reinterpret_cast<list<int>*>(playerResult.pObject));

	// �÷��̾� ���� �Է�
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

	// �ش� ������ ���� ��� �ο����� ����
	list<int> roomMembers = mRoomManager->GetMembersInRoom(roomIndex);
	for (int memberSession : roomMembers)
	{
		SendPacketFunc(memberSession, sizeof(readyPacket), reinterpret_cast<char*>(&readyPacket));
	}

	// �����غ��� �ο��� ���ӿ� �ʿ��� �����ŭ ���� ���� ��
	if (players.size() != pGame->GetMaxPlayerCount())
		return;

	// ���� ���� Ŭ���� �� ����
	pGame->Clear();
	pGame->Start();

	// ���� ��
	mRoomManager->SetPlayStatus(roomIndex, true);

	// ���� ������ �÷��̾� ���� map ȣ��
	map<int, USHORT> playerSession = pGame->GetPlayerSession();

	// ���� ���� ��Ŷ
	GAME_START_PACKET gameStartPacket;
	gameStartPacket.PacketID = (UINT16)PACKET_ID::GAME_START;
	gameStartPacket.PacketLength = sizeof(GAME_START_PACKET);

	// �÷��̾�鿡�� ����
	for (int player : players)
	{
		// map ���� �ش� �÷��̾�� ���õ� ���� ã��
		auto iter = playerSession.find(player);

		// ã�� ���ߴٸ� �������� �̵�
		if (iter == playerSession.end())
			continue;

		// ������ ���� ���� �Է�
		gameStartPacket.omokSide = iter->second;

		SendPacketFunc(player, sizeof(gameStartPacket), reinterpret_cast<char*>(&gameStartPacket));
	}
}

/*
* @brief ��ǥ�� �� �α�
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param packetSize: ��Ŷ ������
* @param pPacket: ��Ŷ ������
* @return void
*/
void PacketManager::ChoicePlace(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pGameTurnReqPacket = reinterpret_cast<GAME_TURN_REQ_PACKET*>(pPacket);

	// �ش� ���� ��ǥ ���� ���
	// ���۷����� �� ����: ����� �� ��ü�� �Ʊ��ٰ� �����߱� ����. ������ �ǵ帮���� �������̱� ������ ������ �����͵� ������� �����̴�.
	GameTurnPlaceInfo& refGameTurnPlaceInfo = pGameTurnReqPacket->placeInfo;

	// ������ �̿��ؼ� ���� ������ ���
	Session* pSession = mSessionManager->GetUserByConnIdx(clientIndex);
	int roomIndex = pSession->GetCurrentRoom();
	Game* pGame = mGameManager->GetGame(roomIndex);

	// ��ǥ�� �� �α�
	COMMON_RESULT_DTO result = pGame->ChoicePlace(clientIndex, refGameTurnPlaceInfo.horizontalNum, refGameTurnPlaceInfo.verticalNum);

	// ���� �δ� �� �����ߴٸ�
	if (result.code != ERROR_CODE::NOTHING)
		return;

	// �ش� ���� ���� ���޹ޱ�. (�ʿ����� �ʴٰ� �����Ѵ�. ref�� �ִ� ���� �״�� ���°ſ� ���������̴�)
	GameTurnPlaceInfo* pResultPlaceInfo = reinterpret_cast<GameTurnPlaceInfo*>(result.pObject);

	// ���� �� ��� ��Ŷ
	GAME_TURN_RESULT_PACKET gameTurnResultPacket;
	gameTurnResultPacket.PacketID = (UINT16)PACKET_ID::GAME_CHOICE_PLACE_RESULT;
	gameTurnResultPacket.PacketLength = sizeof(GAME_TURN_RESULT_PACKET);
	gameTurnResultPacket.result = *pResultPlaceInfo;

	// �÷��̾�鿡�� ����
	map<int, USHORT> playersMap = pGame->GetPlayerSession();
	for (auto iter = playersMap.begin(); iter != playersMap.end(); ++iter)
	{
		SendPacketFunc(iter->first, sizeof(gameTurnResultPacket), reinterpret_cast<char*>(&gameTurnResultPacket));
	}

	// �¸� ���� �Ǵ�
	bool isGameOver = pGame->ConfirmWin(refGameTurnPlaceInfo.omokSide, refGameTurnPlaceInfo.horizontalNum, refGameTurnPlaceInfo.verticalNum);

	// ���� �¸����� ���ߴٸ�
	if (isGameOver == false)
		return;

	// �¸������� ���ӿ���
	GAME_OVER_PACKET gameOverPacket;
	gameOverPacket.PacketID = (UINT16)PACKET_ID::GAME_OVER;
	gameOverPacket.PacketLength = sizeof(gameOverPacket);
	gameOverPacket.winnerSide = refGameTurnPlaceInfo.omokSide;

	for (auto iter = playersMap.begin(); iter != playersMap.end(); ++iter)
	{
		SendPacketFunc(iter->first, sizeof(gameOverPacket), reinterpret_cast<char*>(&gameOverPacket));
	}

	// ���� �÷��̾� ����Ʈ ����
	pGame->ResetPlayers();

	// ���� ���� �ƴ�
	mRoomManager->SetPlayStatus(roomIndex, false);
}

/*
* @brief ���� ����
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param code: ���� �ڵ�
* @return void
*/
void PacketManager::ErrorSend(UINT32 clientIndex, ERROR_CODE code)
{
	// ���� ��Ŷ
	SYS_USER_ERROR_PACKET packet;
	packet.PacketID = (UINT16)PACKET_ID::SYS_ERROR;
	packet.PacketLength = sizeof(SYS_USER_ERROR_PACKET);
	packet.errorCode = static_cast<unsigned short>(code);

	SendPacketFunc(clientIndex, sizeof(packet), reinterpret_cast<char*>(&packet));
}
