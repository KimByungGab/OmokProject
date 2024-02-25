#pragma once

#include "Packet.h"

#pragma pack(push, 1)

struct LOGIN_REQ_PACKET : public PACKET_HEADER
{
	char ID[45];
	char PW[45];
};

struct LOGIN_RES_PACKET : public PACKET_HEADER
{
	bool LoginResult;
};

struct LOGOUT_REQ_PACKET : public PACKET_HEADER
{
};

struct LOGOUT_RES_PACKET : public PACKET_HEADER
{
};

#pragma pack(pop)