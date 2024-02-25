#pragma once

#include "Game.h"
#include "CommonDTO.h"

#include <iostream>
#include <map>
#include <mutex>
#include <utility>

using namespace std;

class GameManager
{
public:
	Game* GetGame(int roomIndex);
	void CreateGame(int roomIndex);
	bool RemoveGame(int roomIndex);
	
	int GetMaxPlayerCount();

private:
	map<int, Game> mGames;
	mutex mLock;
};

