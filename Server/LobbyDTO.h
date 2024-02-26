#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// ��� �� ���� ��û
struct GET_ROOMS_REQ_PACKET : public PACKET_HEADER
{
};

// �κ񿡼� ���� �� ����
struct lobby_room_info
{
	int roomIndex;						// �� �ε���
	WCHAR roomName[45];					// �� �̸�
	unsigned int roomNameSize;			// �� �̸� Length
	unsigned int currentUserCount;		// ���� �ִ� ���� ��
	unsigned int totalUserCount;		// �� �ִ� �ο� ��
};

// ��� �� ���� ����
struct GET_ROOMS_RES_PACKET : public PACKET_HEADER
{
	int totalRoomCount;				// �� �� ���� ��
	lobby_room_info roomInfos[40];	// �� ����
};

// �� ����Ʈ �߰��� ���� ���� ����
struct INSERT_ROOM_LIST_PACKET : public PACKET_HEADER
{
	int index;					// �߰��� ���� ���� �迭 �ε���
	lobby_room_info roomInfo;	// �� ����
};

// �� ����Ʈ ������ ���� ���� ����
struct REMOVE_ROOM_LIST_PACKET : public PACKET_HEADER
{
	int index;	// ������ ���� ���� �迭 �ε���
};

// �濡�� ������ ���� ���� �������� ���� ���� ����
struct UPDATE_ROOM_USER_PACKET : public PACKET_HEADER
{
	int index;					// ���ŵǴ� ���� ���� �迭 �ε���
	UINT currentUserCount;		// �ش� ���� ���� ���� ��
};

#pragma pack(pop)