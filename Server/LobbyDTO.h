#pragma once

#include "Packet.h"

#pragma pack(push, 1)

struct GET_ROOMS_REQ_PACKET : public PACKET_HEADER
{
};

struct lobby_room_info
{
	int roomIndex;
	WCHAR roomName[45];
	unsigned int roomNameSize;
	unsigned int currentUserCount;
	unsigned int totalUserCount;
};
struct GET_ROOMS_RES_PACKET : public PACKET_HEADER
{
	int totalRoomCount;
	lobby_room_info roomInfos[40];
};

struct INSERT_ROOM_LIST_PACKET : public PACKET_HEADER
{
	int index;
	lobby_room_info roomInfo;
};

struct REMOVE_ROOM_LIST_PACKET : public PACKET_HEADER
{
	int index;
};

struct UPDATE_ROOM_USER_PACKET : public PACKET_HEADER
{
	int index;
	UINT currentUserCount;
};

#pragma pack(pop)