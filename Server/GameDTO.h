#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// �غ�Ȯ�� ��û
struct GAME_READY_REQ_PACKET : public PACKET_HEADER
{
};

// �÷��̾� ����
struct PlayerInfo
{
	int idx;
	WCHAR id[45];
	USHORT idSize;
};

// �غ�Ȯ�� ����
struct GAME_READY_RES_PACKET : public PACKET_HEADER
{
	PlayerInfo playerInfos[2];
};

// ���� ����
struct GAME_START_PACKET : public PACKET_HEADER
{
	// �ڽ��� ���� ����
	USHORT omokSide;
};

// ���� �Ͽ� ���� �ξ��� ����
struct GameTurnPlaceInfo
{
	USHORT omokSide;			// �� ����
	USHORT horizontalNum;		// ���� ��ǥ
	USHORT verticalNum;			// ���� ��ǥ
};

// �ڽ��� ���� �� ��ǥ ��û
struct GAME_TURN_REQ_PACKET : public PACKET_HEADER
{
	GameTurnPlaceInfo placeInfo;
};

// ���� �� �� ��ǥ ���
struct GAME_TURN_RESULT_PACKET : public PACKET_HEADER
{
	GameTurnPlaceInfo result;
};

// ���� ��
struct GAME_OVER_PACKET : public PACKET_HEADER
{
	USHORT winnerSide;	// ���� �� ����
};

#pragma pack(pop)