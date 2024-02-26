#pragma once

// COMMON_RESULT_DTO 에러 코드
enum class ERROR_CODE : unsigned short
{
	NOTHING = 0,				// 에러 없음
	
	TOO_MANY_ROOM = 10,			// 방 개수 초과

	TOO_MANY_MEMBER = 20,		// 인원 초과
	WRONG_ROOM = 21,			// 잘못된 방
	NOT_IN_THIS_ROOM = 22,		// 해당 사람이 방 안에 없음

	CANNOT_READY_GAME = 30,		// 게임준비를 할 수 없음
	CANNOT_FIND_ROOM = 31,		// 방을 찾을 수 없음
	NOT_YOUR_TURN = 32,			// 내 턴이 아님
	ALREADY_PLACE = 33,			// 이미 돌을 두었던 자리
	NOT_FOUND_SESSION = 34,		// 세션 인덱스를 찾을 수 없음
};