#include "GameManager.h"

/*
* @brief 게임 정보 호출
* @author 김병갑
* @param roomIndex: 방 인덱스
* @return Game 포인터
*/
Game* GameManager::GetGame(int roomIndex)
{
    auto iter = mGames.find(roomIndex);
    if (iter == mGames.end())
        return nullptr;

    return &(iter->second);
}

/*
* @brief 게임 생성
* @author 김병갑
* @param roomIndex: 방 인덱스
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
* @brief 게임 제거
* @author 김병갑
* @param roomIndex: 방 인덱스
* @return 제거 여부. true면 성공, false면 실패
*/
bool GameManager::RemoveGame(int roomIndex)
{
    lock_guard<mutex> guard(mLock);
    mGames.erase(roomIndex);

    return true;
}