#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// 준비확인 요청
struct GAME_READY_REQ_PACKET : public PACKET_HEADER
{
};

// 플레이어 정보
struct PlayerInfo
{
	int idx;
	WCHAR id[45];
	USHORT idSize;
};

// 준비확인 응답
struct GAME_READY_RES_PACKET : public PACKET_HEADER
{
	PlayerInfo playerInfos[2];
};

// 게임 시작
struct GAME_START_PACKET : public PACKET_HEADER
{
	// 자신의 오목 색깔
	USHORT omokSide;
};

// 게임 턴에 돌을 두었던 정보
struct GameTurnPlaceInfo
{
	USHORT omokSide;			// 돌 색깔
	USHORT horizontalNum;		// 가로 좌표
	USHORT verticalNum;			// 세로 좌표
};

// 자신의 돌을 둔 좌표 요청
struct GAME_TURN_REQ_PACKET : public PACKET_HEADER
{
	GameTurnPlaceInfo placeInfo;
};

// 게임 턴 돌 좌표 결과
struct GAME_TURN_RESULT_PACKET : public PACKET_HEADER
{
	GameTurnPlaceInfo result;
};

// 게임 끝
struct GAME_OVER_PACKET : public PACKET_HEADER
{
	USHORT winnerSide;	// 승자 돌 색깔
};

#pragma pack(pop)