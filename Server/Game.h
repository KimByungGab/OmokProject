#pragma once

#include "CommonDTO.h"
#include "GameDTO.h"

#include <ctime>
#include <cstdlib>
#include <memory>
#include <thread>
#include <list>
#include <mutex>
#include <map>
#include <algorithm>

using namespace std;

#define MAX_PLAYER_COUNT 2		// 최대 플레이어 수
#define MAX_BOARD_COUNT 19		// 오목 보드 줄 최대 수

// 오목 색깔
enum class OMOK_SIDE : USHORT
{
	BLACK = 1,
	WHITE = 2
};

// 게임 정보
class Game
{
public:
	Game();
	Game(const Game& game);
	~Game();

	COMMON_RESULT_DTO Ready(int sessionIndex);
	void Start();
	COMMON_RESULT_DTO ChoicePlace(int sessionIndex, USHORT horizontalNum, USHORT verticalNum);
	bool ConfirmWin(USHORT omokSide, USHORT latestHorizontalNum, USHORT latestVerticalNum);
	void Clear();
	void ResetPlayers();

	int GetMaxPlayerCount();
	list<int> GetReadySession();
	map<int, USHORT> GetPlayerSession();

private:
	int mBlackSideIndex;	// 검정 세션 인덱스
	int mWhiteSideIndex;	// 하양 세션 인덱스
	USHORT mBoard[MAX_BOARD_COUNT][MAX_BOARD_COUNT] = { 0, };	// 오목판 정보. 0이면 없음, 그 외는 OMOK_SIDE 색깔
	OMOK_SIDE mCurrentTurn = OMOK_SIDE::BLACK;					// 현재 턴
	list<int> mPlayers;		// 플레이어 리스트
	mutex mLock;			// 뮤텍스 락
};