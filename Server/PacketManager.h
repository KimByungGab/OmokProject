#pragma once

#include "RoomManager.h"
#include "SessionManager.h"
#include "GameManager.h"
#include "DBConnection.h"

#include "CommonDTO.h"
#include "SystemDTO.h"
#include "LoginDTO.h"
#include "LobbyDTO.h"
#include "RoomDTO.h"
#include "GameDTO.h"

#include <iostream>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <ctime>
#include <cstdlib>

class PacketManager
{
public:
	PacketManager() = default;
	~PacketManager() = default;

	void Init(const UINT32 maxClient);
	void Run();
	void End();
	void PushPacketClient(const UINT32 clientIndex);
	void ReceivePacket(const UINT32 clientIndex, const UINT32 size, char* pData);

	void ConnectClient(UINT32 clientIndex);
	void DisconnectClient(UINT32 clientIndex);

	function<void(UINT32, UINT32, char*)> SendPacketFunc;

	void AddSession(const int addedCount);

private:
	PacketInfo GetPacketData();

	void ProcessPacket();
	void ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetID, const UINT16 packetSize, char* pPacket);

	void LoginUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void LogoutUser(UINT32 clientIndex, UINT16 packetSize, char* pPacket);

	void GetRooms(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void CreateRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void EnterRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket);

	void LeaveRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void ChatRoom(UINT32 clientIndex, UINT16 packetSize, char* pPacket);

	void ReadyGame(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void ChoicePlace(UINT32 clientIndex, UINT16 packetSize, char* pPacket);

	void ErrorSend(UINT32 clientIndex, ERROR_CODE code);

	typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(UINT32, UINT16, char*);
	unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> mRecvFunctionTable;

	queue<UINT32> mClientQueue;
	thread mProcessThread;
	bool mIsProcessRun = false;
	mutex mLock;

	RoomManager* mRoomManager;
	SessionManager* mSessionManager;
	GameManager* mGameManager;
	DBConnection* mDBConn;
};