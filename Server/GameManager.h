#pragma once

#include "Game.h"
#include "CommonDTO.h"

#include <iostream>
#include <map>
#include <mutex>
#include <utility>

using namespace std;

// 게임 매니저
class GameManager
{
public:
	Game* GetGame(int roomIndex);
	void CreateGame(int roomIndex);
	bool RemoveGame(int roomIndex);

private:
	map<int, Game> mGames;	// 게임 Map 
	/*
	* Map 선정 이유 (unordered_map을 사용해도 되겠는데..?)
	* 1. 삽입삭제가 많다: vector X
	* 2. 찾기가 용이해야한다: list X
	* 3. Game 정보로만 찾을 수 없다: Set X
	*/

	mutex mLock;			// 뮤텍스 락
};

