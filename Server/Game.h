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

#define MAX_PLAYER_COUNT 2
#define MAX_BOARD_COUNT 19

enum class OMOK_SIDE : unsigned short
{
	BLACK = 1,
	WHITE = 2
};

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
	int mBlackSideIndex;
	int mWhiteSideIndex;
	unsigned short mBoard[MAX_BOARD_COUNT][MAX_BOARD_COUNT] = { 0, };
	OMOK_SIDE mCurrentTurn = OMOK_SIDE::BLACK;
	list<int> mPlayers;
	mutex mLock;
};