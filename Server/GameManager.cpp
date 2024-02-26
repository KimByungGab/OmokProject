#include "GameManager.h"

/*
* @brief ���� ���� ȣ��
* @author �躴��
* @param roomIndex: �� �ε���
* @return Game ������
*/
Game* GameManager::GetGame(int roomIndex)
{
    auto iter = mGames.find(roomIndex);
    if (iter == mGames.end())
        return nullptr;

    return &(iter->second);
}

/*
* @brief ���� ����
* @author �躴��
* @param roomIndex: �� �ε���
* @return void
*/
void GameManager::CreateGame(int roomIndex)
{
    Game newGame;
    newGame.Clear();

    lock_guard<mutex> guard(mLock);
    mGames.insert(make_pair(roomIndex, newGame));
}

/*
* @brief ���� ����
* @author �躴��
* @param roomIndex: �� �ε���
* @return ���� ����. true�� ����, false�� ����
*/
bool GameManager::RemoveGame(int roomIndex)
{
    lock_guard<mutex> guard(mLock);
    mGames.erase(roomIndex);

    return true;
}