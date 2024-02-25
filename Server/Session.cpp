#include "Session.h"

void Session::Init(const int clientIndex, const char* id)
{
	mIndex = clientIndex;
	mID = string(id);
}

void Session::Clear()
{
	mRoomIndex = -1;
	mCurrentLocation = LOCATION::NONE;

	mPacketDataBufferWPos = 0;
	mPacketDataBufferRPos = 0;
}

void Session::EnterLobby()
{
	mRoomIndex = -1;
	mCurrentLocation = LOCATION::LOBBY;
}

void Session::EnterRoom(int roomIndex)
{
	mRoomIndex = roomIndex;
	mCurrentLocation = LOCATION::ROOM;
}

Session::LOCATION Session::GetCurrentLocation()
{
	return mCurrentLocation;
}

void Session::SetCurrentLocation(LOCATION location)
{
	mCurrentLocation = location;
}

int Session::GetCurrentRoom()
{
	return mRoomIndex;
}

int Session::GetConnIdx()
{
	return mIndex;
}

void Session::SetId(string id)
{
	mID = id;
}

string Session::GetId()
{
	return mID;
}

void Session::SetIdx(int idx)
{
	mIdx = idx;
}

int Session::GetIdx()
{
	return mIdx;
}

PacketInfo Session::GetPacket()
{
	int remainByte = mPacketDataBufferWPos - mPacketDataBufferRPos;

	if (remainByte < PACKET_HEADER_LENGTH)
		return PacketInfo();

	auto pHeader = (PACKET_HEADER*)&mPacketDataBuffer[mPacketDataBufferRPos];
	if (pHeader->PacketLength > remainByte)
		return PacketInfo();

	PacketInfo packetInfo;
	packetInfo.PacketID = pHeader->PacketID;
	packetInfo.DataSize = pHeader->PacketLength;
	packetInfo.pDataPtr = &mPacketDataBuffer[mPacketDataBufferRPos];

	mPacketDataBufferRPos += pHeader->PacketLength;

	return packetInfo;
}

void Session::SetPacketData(const int dataSize, char* pData)
{
	if ((mPacketDataBufferWPos + dataSize) >= PACKET_DATA_BUFFER_SIZE)
	{
		auto remainDataSize = mPacketDataBufferWPos - mPacketDataBufferRPos;
		if (remainDataSize > 0)
		{
			memcpy(&mPacketDataBuffer[0], &mPacketDataBuffer[mPacketDataBufferRPos], remainDataSize);
			mPacketDataBufferWPos = remainDataSize;
		}
		else
		{
			mPacketDataBufferWPos = 0;
		}

		mPacketDataBufferRPos = 0;
	}

	memcpy(&mPacketDataBuffer[mPacketDataBufferWPos], pData, dataSize);
	mPacketDataBufferWPos += dataSize;
}
