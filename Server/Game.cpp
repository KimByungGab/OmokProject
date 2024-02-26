#include "Game.h"

Game::Game()
{
}

// 게임을 만들 때 복사생성자를 이용하기 때문에 만들어둠
Game::Game(const Game& game)
{
}

Game::~Game()
{
}

/*
* @brief 게임 준비 혹은 준비취소
* @author 김병갑
* @param sessionIndex: 세션 인덱스 번호
* @return Common result DTO
*/
COMMON_RESULT_DTO Game::Ready(int sessionIndex)
{
	// return DTO
	COMMON_RESULT_DTO retVal;

	// 현재 준비한 플레이어 중에서 본인이 포함되어있는지 찾기
	auto iter = find_if(mPlayers.begin(), mPlayers.end(), [=](int playerIndex)
		{
			return sessionIndex == playerIndex;
		});

	// 만약 있다면 준비해제
	if (iter != mPlayers.end())
	{
		lock_guard<mutex> guard(mLock);
		mPlayers.erase(iter);
	}

	// 이미 플레이어가 정원초과되었다면
	else if (mPlayers.size() == MAX_PLAYER_COUNT)
	{
		retVal.code = ERROR_CODE::CANNOT_READY_GAME;
		return retVal;
	}

	// 위의 둘이 아닌 경우에는 정상적으로 준비
	else
	{
		lock_guard<mutex> guard(mLock);
		mPlayers.push_back(sessionIndex);
	}

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<char*>(&mPlayers);
	
	return retVal;
}

/*
* @brief 게임 시작
* @author 김병갑
* @return void
*/
void Game::Start()
{
	// 랜덤값때문에 시드 설정
	srand(static_cast<unsigned int>(time(NULL)));

	// 랜덤하게 값을 받음
	int oneRandNum = rand();
	int twoRandNum = rand();

	// 랜덤한 값이 큰 사람이 검정색이 됨
	// map에서 두명밖에 하지 않으니 front back으로 처리함
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

/*
* @brief 바둑돌 처리
* @author 김병갑
* @param sessionIndex: 바둑돌을 둔 세션 인덱스
* @param horizontalNum: 바둑돌 가로 좌표
* @param verticalNum: 바둑돌 세로 좌표
* @return Common result DTO
*/
COMMON_RESULT_DTO Game::ChoicePlace(int sessionIndex, USHORT horizontalNum, USHORT verticalNum)
{
	COMMON_RESULT_DTO retVal;					// return DTO
	OMOK_SIDE nextSide = OMOK_SIDE::BLACK;		// 다음 턴 색깔. 초기화는 의미없다. visual studio에서 밑줄쳐서 한 것이다.

	// 현재 턴에 따라 다르게 적용됨
	switch (mCurrentTurn)
	{
	case OMOK_SIDE::BLACK:

		// 만약 본인 턴이 아닌 경우
		if (mBlackSideIndex != sessionIndex)
		{
			retVal.code = ERROR_CODE::NOT_YOUR_TURN;
			return retVal;
		}

		// 아무 돌도 없는 곳이라면
		if (mBoard[horizontalNum][verticalNum] == 0)
		{
			mBoard[horizontalNum][verticalNum] = static_cast<unsigned short>(OMOK_SIDE::BLACK);
			nextSide = OMOK_SIDE::WHITE;
		}

		// 이미 두었던 곳은 에러처리
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

	// 현재 턴의 결과 struct
	GameTurnPlaceInfo result;
	result.omokSide = static_cast<USHORT>(mCurrentTurn);
	result.horizontalNum = horizontalNum;
	result.verticalNum = verticalNum;

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<char*>(&result);

	// 다음 턴 적용
	mCurrentTurn = nextSide;

	return retVal;
}

/*
* @brief 승리 여부 확인
* @author 김병갑
* @param omokSide: 오목 색깔
* @param latestHorizontalNum: 최근 두었던 수의 가로 좌표
* @param latestVerticalNum: 최근 두었던 수의 세로 좌표
* @return 승리 여부. true면 승리, false면 승리아님
*/
bool Game::ConfirmWin(USHORT omokSide, USHORT latestHorizontalNum, USHORT latestVerticalNum)
{
	// 오목이기 때문에 자신이 최근에 두었던 범위에서 4만큼의 범위만큼 계산을 한다.
	// 가장 최근에 두었던 수(1) + 4 = 5

	// 가로 확인
	int sideCount = 0;	// 연속적으로 측정된 돌 개수, 조건이 하나라도 맞지 않으면 0으로 초기화된다.

	// 오목이 될 수 있는 범위를 확인
	for (int i = latestHorizontalNum - 4; i <= latestHorizontalNum + 4; i++)
	{
		// 만약 범위를 넘어섰다면
		if (i < 0 || i >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		// 확인결과 내가 두었던 돌이라면
		if (mBoard[i][latestVerticalNum] == omokSide)
			sideCount++;
		else
			sideCount = 0;

		// 오목 완성 되었다면
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

	// 모든 경우에도 해당되지 않으면
	return false;
}

/*
* @brief 게임 초기 세팅
* @author 김병갑
* @return void
*/
void Game::Clear()
{
	// session index 초기화
	mBlackSideIndex = -1;
	mWhiteSideIndex = -1;

	// 좌표에 있는 돌 전부 0으로 세팅
	memset(mBoard, 0, sizeof(mBoard));

	// 시작은 무조건 검정색이 먼저 시작
	mCurrentTurn = OMOK_SIDE::BLACK;
}

/*
* @brief 플레이어 리스트 초기화
* @author 김병갑
* @return void
*/
void Game::ResetPlayers()
{
	mPlayers.clear();
}

/*
* @brief 게임 최대 인원수 호출
* @author 김병갑
* @return 게임 최대 인원수
*/
int Game::GetMaxPlayerCount()
{
	return MAX_PLAYER_COUNT;
}

/*
* @brief 준비한 세션 리스트 호출
* @author 김병갑
* @return 세션 리스트
*/
list<int> Game::GetReadySession()
{
	return mPlayers;
}

/*
* @brief 게임중인 플레이어 호출
* @author 김병갑
* @return 플레이어 정보 map (map<세션 인덱스, 검정 or 하양>)
*/
map<int, USHORT> Game::GetPlayerSession()
{
	// 결과
	map<int, USHORT> retVal;

	// 세션 인덱스와 색깔을 map에 insert
	retVal.insert(make_pair(mBlackSideIndex, static_cast<USHORT>(OMOK_SIDE::BLACK)));
	retVal.insert(make_pair(mWhiteSideIndex, static_cast<USHORT>(OMOK_SIDE::WHITE)));

	return retVal;
}
