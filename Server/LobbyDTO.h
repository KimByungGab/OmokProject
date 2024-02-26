#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// 모든 방 정보 요청
struct GET_ROOMS_REQ_PACKET : public PACKET_HEADER
{
};

// 로비에서 보는 방 정보
struct lobby_room_info
{
	int roomIndex;						// 방 인덱스
	WCHAR roomName[45];					// 방 이름
	unsigned int roomNameSize;			// 방 이름 Length
	unsigned int currentUserCount;		// 현재 있는 유저 수
	unsigned int totalUserCount;		// 방 최대 인원 수
};

// 모든 방 정보 응답
struct GET_ROOMS_RES_PACKET : public PACKET_HEADER
{
	int totalRoomCount;				// 총 방 정보 수
	lobby_room_info roomInfos[40];	// 방 정보
};

// 방 리스트 추가로 인한 갱신 응답
struct INSERT_ROOM_LIST_PACKET : public PACKET_HEADER
{
	int index;					// 추가될 방의 실제 배열 인덱스
	lobby_room_info roomInfo;	// 방 정보
};

// 방 리스트 삭제로 인한 갱신 응답
struct REMOVE_ROOM_LIST_PACKET : public PACKET_HEADER
{
	int index;	// 삭제될 방의 셀제 배열 인덱스
};

// 방에서 입장한 유저 수의 변경으로 인한 갱신 응답
struct UPDATE_ROOM_USER_PACKET : public PACKET_HEADER
{
	int index;					// 갱신되는 방의 실제 배열 인덱스
	UINT currentUserCount;		// 해당 방의 현재 유저 수
};

#pragma pack(pop)