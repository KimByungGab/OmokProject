#pragma once

#include "Packet.h"

#define PACKET_DATA_BUFFER_SIZE 8096	// ��Ŷ ���� ������

// ���� Ŭ����
class Session
{
public:
	
	// ��ġ enum class
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
	int mIndex = -1;			// ���� �ε���
	int mRoomIndex = -1;		// �� �� ��ȣ

	LOCATION mCurrentLocation = LOCATION::NONE;		// ���� ��ġ

	string mID;		// ���̵�
	int mIdx;		// DB idx ��ȣ

	int mPacketDataBufferWPos = 0;									// ������ �� ��ġ
	int mPacketDataBufferRPos = 0;									// ������ ���� ��ġ
	char mPacketDataBuffer[PACKET_DATA_BUFFER_SIZE] = { 0, };		// ��Ŷ ����
};

