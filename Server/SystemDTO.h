#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// 유저 연결
struct SYS_USER_CONNECT_PACKET : public PACKET_HEADER
{
};

// 유저 연결해제
struct SYS_USER_DISCONNECT_PACKET : public PACKET_HEADER
{
};

// 에러 응답
struct SYS_USER_ERROR_PACKET : public PACKET_HEADER
{
	unsigned short errorCode;	// 에러 코드(ErrorCode.h)
};

#pragma pack(pop)