#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// �� ���� ��û
struct ROOM_CREATE_REQ_PACKET : public PACKET_HEADER
{
	// �� �̸�
	WCHAR roomName[45];
};

// �� ���� ����
struct ROOM_CREATE_RES_PACKET : public PACKET_HEADER
{
	// ������� �� �ε���
	int roomIndex;
};

// �� ���� ��û
struct ROOM_ENTER_REQ_PACKET : public PACKET_HEADER
{
	// ������ �� �ε���
	int roomIndex;
};

// �� ���� ����
struct ROOM_ENTER_RES_PACKET : public PACKET_HEADER
{
};

// �� ���� ��û
struct ROOM_LEAVE_REQ_PACKET : public PACKET_HEADER
{
};

// �� ���� ����
struct ROOM_LEAVE_RES_PACKET : public PACKET_HEADER
{
};

// ä�� ��û
struct ROOM_CHAT_REQ_PACKET : public PACKET_HEADER
{
	// ä�� ����
	WCHAR chatData[MAX_CHAT_BUFFER];
};

// ä�� ����
struct ROOM_CHAT_RES_PACKET : public PACKET_HEADER
{
	WCHAR chatData[MAX_CHAT_BUFFER];	// ä�� ����
	UINT chatDataSize;					// ä�� ���� Length
};

#pragma pack(pop)