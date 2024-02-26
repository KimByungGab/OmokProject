#pragma once

#include "Packet.h"

#pragma pack(push, 1)

// 로그인 요청
struct LOGIN_REQ_PACKET : public PACKET_HEADER
{
	char ID[45];
	char PW[45];
};

// 로그인 응답
struct LOGIN_RES_PACKET : public PACKET_HEADER
{
	// 로그인 결과. true면 성공, false면 실패
	bool LoginResult;
};

// 로그아웃 요청
struct LOGOUT_REQ_PACKET : public PACKET_HEADER
{
};

// 로그아웃 응답
struct LOGOUT_RES_PACKET : public PACKET_HEADER
{
};

#pragma pack(pop)