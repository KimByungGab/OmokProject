#pragma once

#include "Packet.h"

#pragma pack(push, 1)

struct SYS_USER_CONNECT_PACKET : public PACKET_HEADER
{
};

struct SYS_USER_DISCONNECT_PACKET : public PACKET_HEADER
{
};

struct SYS_USER_ERROR_PACKET : public PACKET_HEADER
{
	unsigned short errorCode;
};

#pragma pack(pop)