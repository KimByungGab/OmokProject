#pragma once

#include "Packet.h"

#define PACKET_DATA_BUFFER_SIZE 8096	// 패킷 버퍼 사이즈

// 세션 클래스
class Session
{
public:
	
	// 위치 enum class
	enum class LOCATION
	{
		NONE,
		LOBBY,
		ROOM
	};

	Session() = default;
	~Session() = default;

	void Init(const int clientIndex, const char* id);
	void Clear();
	void EnterLobby();
	void EnterRoom(int roomIndex);
	Session::LOCATION GetCurrentLocation();
	void SetCurrentLocation(LOCATION location);
	int GetCurrentRoom();
	int GetConnIdx();
	void SetId(string id);
	string GetId();
	void SetIdx(int idx);
	int GetIdx();
	
	PacketInfo GetPacket();
	void SetPacketData(const int dataSize, char* pData);

private:
	int mIndex = -1;			// 세션 인덱스
	int mRoomIndex = -1;		// 들어간 방 번호

	LOCATION mCurrentLocation = LOCATION::NONE;		// 현재 위치

	string mID;		// 아이디
	int mIdx;		// DB idx 번호

	int mPacketDataBufferWPos = 0;									// 버퍼의 쓴 위치
	int mPacketDataBufferRPos = 0;									// 버퍼의 읽은 위치
	char mPacketDataBuffer[PACKET_DATA_BUFFER_SIZE] = { 0, };		// 패킷 버퍼
};

