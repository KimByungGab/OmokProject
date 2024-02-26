#include "Game.h"

Game::Game()
{
}

// ������ ���� �� ��������ڸ� �̿��ϱ� ������ ������
Game::Game(const Game& game)
{
}

Game::~Game()
{
}

/*
* @brief ���� �غ� Ȥ�� �غ����
* @author �躴��
* @param sessionIndex: ���� �ε��� ��ȣ
* @return Common result DTO
*/
COMMON_RESULT_DTO Game::Ready(int sessionIndex)
{
	// return DTO
	COMMON_RESULT_DTO retVal;

	// ���� �غ��� �÷��̾� �߿��� ������ ���ԵǾ��ִ��� ã��
	auto iter = find_if(mPlayers.begin(), mPlayers.end(), [=](int playerIndex)
		{
			return sessionIndex == playerIndex;
		});

	// ���� �ִٸ� �غ�����
	if (iter != mPlayers.end())
	{
		lock_guard<mutex> guard(mLock);
		mPlayers.erase(iter);
	}

	// �̹� �÷��̾ �����ʰ��Ǿ��ٸ�
	else if (mPlayers.size() == MAX_PLAYER_COUNT)
	{
		retVal.code = ERROR_CODE::CANNOT_READY_GAME;
		return retVal;
	}

	// ���� ���� �ƴ� ��쿡�� ���������� �غ�
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
* @brief ���� ����
* @author �躴��
* @return void
*/
void Game::Start()
{
	// ������������ �õ� ����
	srand(static_cast<unsigned int>(time(NULL)));

	// �����ϰ� ���� ����
	int oneRandNum = rand();
	int twoRandNum = rand();

	// ������ ���� ū ����� �������� ��
	// map���� �θ�ۿ� ���� ������ front back���� ó����
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
* @brief �ٵϵ� ó��
* @author �躴��
* @param sessionIndex: �ٵϵ��� �� ���� �ε���
* @param horizontalNum: �ٵϵ� ���� ��ǥ
* @param verticalNum: �ٵϵ� ���� ��ǥ
* @return Common result DTO
*/
COMMON_RESULT_DTO Game::ChoicePlace(int sessionIndex, USHORT horizontalNum, USHORT verticalNum)
{
	COMMON_RESULT_DTO retVal;					// return DTO
	OMOK_SIDE nextSide = OMOK_SIDE::BLACK;		// ���� �� ����. �ʱ�ȭ�� �ǹ̾���. visual studio���� �����ļ� �� ���̴�.

	// ���� �Ͽ� ���� �ٸ��� �����
	switch (mCurrentTurn)
	{
	case OMOK_SIDE::BLACK:

		// ���� ���� ���� �ƴ� ���
		if (mBlackSideIndex != sessionIndex)
		{
			retVal.code = ERROR_CODE::NOT_YOUR_TURN;
			return retVal;
		}

		// �ƹ� ���� ���� ���̶��
		if (mBoard[horizontalNum][verticalNum] == 0)
		{
			mBoard[horizontalNum][verticalNum] = static_cast<unsigned short>(OMOK_SIDE::BLACK);
			nextSide = OMOK_SIDE::WHITE;
		}

		// �̹� �ξ��� ���� ����ó��
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

	// ���� ���� ��� struct
	GameTurnPlaceInfo result;
	result.omokSide = static_cast<USHORT>(mCurrentTurn);
	result.horizontalNum = horizontalNum;
	result.verticalNum = verticalNum;

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<char*>(&result);

	// ���� �� ����
	mCurrentTurn = nextSide;

	return retVal;
}

/*
* @brief �¸� ���� Ȯ��
* @author �躴��
* @param omokSide: ���� ����
* @param latestHorizontalNum: �ֱ� �ξ��� ���� ���� ��ǥ
* @param latestVerticalNum: �ֱ� �ξ��� ���� ���� ��ǥ
* @return �¸� ����. true�� �¸�, false�� �¸��ƴ�
*/
bool Game::ConfirmWin(USHORT omokSide, USHORT latestHorizontalNum, USHORT latestVerticalNum)
{
	// �����̱� ������ �ڽ��� �ֱٿ� �ξ��� �������� 4��ŭ�� ������ŭ ����� �Ѵ�.
	// ���� �ֱٿ� �ξ��� ��(1) + 4 = 5

	// ���� Ȯ��
	int sideCount = 0;	// ���������� ������ �� ����, ������ �ϳ��� ���� ������ 0���� �ʱ�ȭ�ȴ�.

	// ������ �� �� �ִ� ������ Ȯ��
	for (int i = latestHorizontalNum - 4; i <= latestHorizontalNum + 4; i++)
	{
		// ���� ������ �Ѿ�ٸ�
		if (i < 0 || i >= MAX_BOARD_COUNT)
		{
			sideCount = 0;
			continue;
		}

		// Ȯ�ΰ�� ���� �ξ��� ���̶��
		if (mBoard[i][latestVerticalNum] == omokSide)
			sideCount++;
		else
			sideCount = 0;

		// ���� �ϼ� �Ǿ��ٸ�
		if (sideCount == 5)
			return true;
	}

	// ���� Ȯ��
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

	// �밢��(������) Ȯ��
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
	
	// �밢��(�����) Ȯ��
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

	// ��� ��쿡�� �ش���� ������
	return false;
}

/*
* @brief ���� �ʱ� ����
* @author �躴��
* @return void
*/
void Game::Clear()
{
	// session index �ʱ�ȭ
	mBlackSideIndex = -1;
	mWhiteSideIndex = -1;

	// ��ǥ�� �ִ� �� ���� 0���� ����
	memset(mBoard, 0, sizeof(mBoard));

	// ������ ������ �������� ���� ����
	mCurrentTurn = OMOK_SIDE::BLACK;
}

/*
* @brief �÷��̾� ����Ʈ �ʱ�ȭ
* @author �躴��
* @return void
*/
void Game::ResetPlayers()
{
	mPlayers.clear();
}

/*
* @brief ���� �ִ� �ο��� ȣ��
* @author �躴��
* @return ���� �ִ� �ο���
*/
int Game::GetMaxPlayerCount()
{
	return MAX_PLAYER_COUNT;
}

/*
* @brief �غ��� ���� ����Ʈ ȣ��
* @author �躴��
* @return ���� ����Ʈ
*/
list<int> Game::GetReadySession()
{
	return mPlayers;
}

/*
* @brief �������� �÷��̾� ȣ��
* @author �躴��
* @return �÷��̾� ���� map (map<���� �ε���, ���� or �Ͼ�>)
*/
map<int, USHORT> Game::GetPlayerSession()
{
	// ���
	map<int, USHORT> retVal;

	// ���� �ε����� ������ map�� insert
	retVal.insert(make_pair(mBlackSideIndex, static_cast<USHORT>(OMOK_SIDE::BLACK)));
	retVal.insert(make_pair(mWhiteSideIndex, static_cast<USHORT>(OMOK_SIDE::WHITE)));

	return retVal;
}
