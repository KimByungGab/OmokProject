#include "GameManager.h"

Game* GameManager::GetGame(int roomIndex)
{
    auto iter = mGames.find(roomIndex);
    if (iter == mGames.end())
        return nullptr;

    return &(iter->second);
}

void GameManager::CreateGame(int roomIndex)
{
    Game newGame;
    newGame.Clear();

    lock_guard<mutex> guard(mLock);
    mGames.insert(make_pair(roomIndex, newGame));
}

bool GameManager::RemoveGame(int roomIndex)
{
    lock_guard<mutex> guard(mLock);
    mGames.erase(roomIndex);

    return true;
}

int GameManager::GetMaxPlayerCount()
{
    Game game;
    return game.GetMaxPlayerCount();
}
