#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// 방 생성 요청
struct ROOM_CREATE_REQ_PACKET : public PACKET_HEADER
{
	// 방 이름
	WCHAR roomName[45];
};

// 방 생성 응답
struct ROOM_CREATE_RES_PACKET : public PACKET_HEADER
{
	// 만들어진 방 인덱스
	int roomIndex;
};

// 방 입장 요청
struct ROOM_ENTER_REQ_PACKET : public PACKET_HEADER
{
	// 입장한 방 인덱스
	int roomIndex;
};

// 방 입장 응답
struct ROOM_ENTER_RES_PACKET : public PACKET_HEADER
{
};

// 방 퇴장 요청
struct ROOM_LEAVE_REQ_PACKET : public PACKET_HEADER
{
};

// 방 퇴장 응답
struct ROOM_LEAVE_RES_PACKET : public PACKET_HEADER
{
};

// 채팅 요청
struct ROOM_CHAT_REQ_PACKET : public PACKET_HEADER
{
	// 채팅 내용
	WCHAR chatData[MAX_CHAT_BUFFER];
};

// 채팅 응답
struct ROOM_CHAT_RES_PACKET : public PACKET_HEADER
{
	WCHAR chatData[MAX_CHAT_BUFFER];	// 채팅 내용
	UINT chatDataSize;					// 채팅 내용 Length
};

#pragma pack(pop)