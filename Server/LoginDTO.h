#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// �α��� ��û
struct LOGIN_REQ_PACKET : public PACKET_HEADER
{
	char ID[45];
	char PW[45];
};

// �α��� ����
struct LOGIN_RES_PACKET : public PACKET_HEADER
{
	// �α��� ���. true�� ����, false�� ����
	bool LoginResult;
};

// �α׾ƿ� ��û
struct LOGOUT_REQ_PACKET : public PACKET_HEADER
{
};

// �α׾ƿ� ����
struct LOGOUT_RES_PACKET : public PACKET_HEADER
{
};

#pragma pack(pop)