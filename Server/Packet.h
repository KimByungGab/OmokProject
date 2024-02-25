#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

using namespace std;

#define MAX_CHAT_BUFFER 512

struct PacketInfo
{
	UINT32 ClientIndex = 0;
	UINT16 PacketID = 0;
	UINT16 DataSize = 0;
	char* pDataPtr = nullptr;
};

enum class PACKET_ID : UINT16
{
	// 시스템
	SYS_END = 0,
	SYS_USER_CONNECT = 1,
	SYS_USER_DISCONNECT = 2,
	SYS_ERROR = 3,

	// 로그인
	LOGIN_REQ = 10,
	LOGIN_RES = 11,
	LOGOUT_REQ = 12,
	LOGOUT_RES = 13,

	// 로비
	GET_ROOMS_REQ = 20,
	GET_ROOMS_RES = 21,
	INSERT_ROOM = 22,
	REMOVE_ROOM = 23,
	UPDATE_ROOM = 24,

	// 방
	ROOM_CREATE_REQ = 30,
	ROOM_CREATE_RES = 31,
	ROOM_ENTER_REQ = 32,
	ROOM_ENTER_RES = 33,
	ROOM_LEAVE_REQ = 34,
	ROOM_LEAVE_RES = 35,
	ROOM_CHAT_REQ = 36,
	ROOM_CHAT_RES = 37,

	// 게임
	GAME_READY_REQ = 40,
	GAME_READY_RES = 41,
	GAME_START = 42,
	GAME_CHOICE_PLACE_REQ = 43,
	GAME_CHOICE_PLACE_RESULT = 44,
	GAME_OVER = 45,
};

#pragma pack(push, 1)

struct PACKET_HEADER
{
	UINT16 PacketID;
	UINT16 PacketLength;
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);

#pragma pack(pop)