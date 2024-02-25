#pragma once

#include "Packet.h"

#define PACKET_DATA_BUFFER_SIZE 8096

class Session
{
public:
	
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
	int mIndex = -1;
	int mChannelIndex = -1;
	int mRoomIndex = -1;

	LOCATION mCurrentLocation = LOCATION::NONE;

	string mID;
	int mIdx;

	int mPacketDataBufferWPos = 0;
	int mPacketDataBufferRPos = 0;
	char mPacketDataBuffer[PACKET_DATA_BUFFER_SIZE] = { 0, };
};

