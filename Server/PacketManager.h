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

// 패킷 매니저 클래스
class PacketManager
{
public:
	PacketManager() = default;
	~PacketManager() = default;

	void Init();
	void Run();
	void End();
	void PushPacketClient(const UINT32 clientIndex);
	void ReceivePacket(const UINT32 clientIndex, const UINT32 size, char* pData);

	void ConnectClient(UINT32 clientIndex);
	void DisconnectClient(UINT32 clientIndex);

	function<void(UINT32, UINT32, char*)> SendPacketFunc;	// 패킷 송신 함수 변수

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

	typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(UINT32, UINT16, char*);		// 연결되는 함수 포인터
	unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> mRecvFunctionTable;					// 패킷ID와 함수포인터의 해시테이블

	queue<UINT32> mClientQueue;			// 요청한 클라이언트 큐
	thread mProcessThread;				// 요청 처리하는 쓰레드
	bool mIsProcessRun = false;			// mProcessThread의 무한반복 조건
	mutex mLock;						// 뮤텍스 락

	RoomManager* mRoomManager;			// 방 매니저
	SessionManager* mSessionManager;	// 세션 매니저
	GameManager* mGameManager;			// 게임 매니저
	DBConnection* mDBConn;				// DB 연결 매니저
};