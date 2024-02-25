#pragma once

#include "Packet.h"

#pragma pack(push, 1)

struct ROOM_CREATE_REQ_PACKET : public PACKET_HEADER
{
	WCHAR roomName[45];
};

struct ROOM_CREATE_RES_PACKET : public PACKET_HEADER
{
	int roomIndex;
};

struct ROOM_ENTER_REQ_PACKET : public PACKET_HEADER
{
	int roomIndex;
};

struct ROOM_ENTER_RES_PACKET : public PACKET_HEADER
{
};

struct ROOM_LEAVE_REQ_PACKET : public PACKET_HEADER
{
};

struct ROOM_LEAVE_RES_PACKET : public PACKET_HEADER
{
};

struct ROOM_CHAT_REQ_PACKET : public PACKET_HEADER
{
	WCHAR chatData[MAX_CHAT_BUFFER];
};

struct ROOM_CHAT_RES_PACKET : public PACKET_HEADER
{
	WCHAR chatData[MAX_CHAT_BUFFER];
	UINT chatDataSize;
};

#pragma pack(pop)