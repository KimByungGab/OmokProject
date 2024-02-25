#include "Game.h"

Game::Game()
{
}

Game::Game(const Game& game)
{
}

Game::~Game()
{
}

COMMON_RESULT_DTO Game::Ready(int sessionIndex)
{
	COMMON_RESULT_DTO retVal;

	auto iter = find_if(mPlayers.begin(), mPlayers.end(), [=](int playerIndex)
		{
			return sessionIndex == playerIndex;
		});

	if (iter != mPlayers.end())
	{
		lock_guard<mutex> guard(mLock);
		mPlayers.erase(iter);
	}
	else if (mPlayers.size() == MAX_PLAYER_COUNT)
	{
		retVal.code = ERROR_CODE::CANNOT_READY_GAME;
		return retVal;
	}
	else
	{
		lock_guard<mutex> guard(mLock);
		mPlayers.push_back(sessionIndex);
	}

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<char*>(&mPlayers);
	
	return retVal;
}

void Game::Start()
{
	srand(static_cast<unsigned int>(time(NULL)));

	int oneRandNum = rand();
	int twoRandNum = rand();

	if (oneRandNum >= twoRandNum)
	{
		mBlackSideIndex = mPlayers.front();
		mWhiteSideIndex = mPlayers.back();
	}
	else
	{
		mBlackSideIndex = mPlayers.back();
		mWhiteSideIndex = mPlayers.front();
	}
}

COMMON_RESULT_DTO Game::ChoicePlace(int sessionIndex, USHORT horizontalNum, USHORT verticalNum)
{
	COMMON_RESULT_DTO retVal;
	OMOK_SIDE nextSide = OMOK_SIDE::BLACK;

	switch (mCurrentTurn)
	{
	case OMOK_SIDE::BLACK:
		if (mBlackSideIndex != sessionIndex)
		{
			retVal.code = ERROR_CODE::NOT_YOUR_TURN;
			return retVal;
		}

		if (mBoard[horizontalNum][verticalNum] == 0)
		{
			mBoard[horizontalNum][verticalNum] = static_cast<unsigned short>(OMOK_SIDE::BLACK);
			nextSide = OMOK_SIDE::WHITE;
		}
		else
		{
			retVal.code = ERROR_CODE::ALREADY_PLACE;
			return retVal;
		}

		break;

	case OMOK_SIDE::WHITE:
		if (mWhiteSideIndex != sessionIndex)
		{
			retVal.code = ERROR_CODE::NOT_YOUR_TURN;
			return retVal;
		}

		if (mBoard[horizontalNum][verticalNum] == 0)
		{
			mBoard[horizontalNum][verticalNum] = static_cast<unsigned short>(OMOK_SIDE::WHITE);
			nextSide = OMOK_SIDE::BLACK;
		}
		else
		{
			retVal.code = ERROR_CODE::ALREADY_PLACE;
			return retVal;
		}
		
		break;
	}

	GameTurnPlaceInfo result;
	result.omokSide = static_cast<UINT>(mCurrentTurn);
	result.horizontalNum = horizontalNum;
	result.verticalNum = verticalNum;

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<char*>(&result);

	mCurrentTurn = nextSide;

	return retVal;
}

bool Game::ConfirmWin(USHORT omokSide, USHORT latestHorizontalNum, USHORT latestVerticalNum)
{
	// 가로 확인
	int sideCount = 0;
	for (int i = latestHorizontalNum - 4; i <= latestHorizontalNum + 4; i++)
	{
		if (i < 0 || i >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		if (mBoard[i][latestVerticalNum] == omokSide)
			sideCount++;
		else
			sideCount = 0;

		if (sideCount == 5)
			return true;
	}

	// 세로 확인
	sideCount = 0;
	for (int i = latestVerticalNum - 4; i <= latestVerticalNum + 4; i++)
	{
		if (i < 0 || i >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		if (mBoard[latestHorizontalNum][i] == omokSide)
			sideCount++;
		else
			sideCount = 0;

		if (sideCount == 5)
			return true;
	}

	// 대각선(우하향) 확인
	sideCount = 0;
	for (int i = -4; i <= 4; i++)
	{
		int horizontalNum = latestHorizontalNum + i;
		int verticalNum = latestVerticalNum + i;

		if (horizontalNum < 0 || horizontalNum >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}
		
		if (verticalNum < 0 || verticalNum >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		if (mBoard[horizontalNum][verticalNum] == omokSide)
			sideCount++;
		else
			sideCount = 0;

		if (sideCount == 5)
			return true;
	}
	
	// 대각선(우상향) 확인
	sideCount = 0;
	for (int i = -4; i <= 4; i++)
	{
		int horizontalNum = latestHorizontalNum + i;
		int verticalNum = latestVerticalNum - i;

		if (horizontalNum < 0 || horizontalNum >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		if (verticalNum < 0 || verticalNum >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		if (mBoard[horizontalNum][verticalNum] == omokSide)
			sideCount++;
		else
			sideCount = 0;

		if (sideCount == 5)
			return true;
	}

	return false;
}

void Game::Clear()
{
	mBlackSideIndex = -1;
	mWhiteSideIndex = -1;

	memset(mBoard, 0, sizeof(mBoard));

	mCurrentTurn = OMOK_SIDE::BLACK;
}

void Game::ResetPlayers()
{
	mPlayers.clear();
}

int Game::GetMaxPlayerCount()
{
	return MAX_PLAYER_COUNT;
}

list<int> Game::GetReadySession()
{
	return mPlayers;
}

map<int, USHORT> Game::GetPlayerSession()
{
	map<int, USHORT> retVal;

	retVal.insert(make_pair(mBlackSideIndex, static_cast<USHORT>(OMOK_SIDE::BLACK)));
	retVal.insert(make_pair(mWhiteSideIndex, static_cast<USHORT>(OMOK_SIDE::WHITE)));

	return retVal;
}
