#pragma once

#include "Game.h"
#include "CommonDTO.h"

#include <iostream>
#include <map>
#include <mutex>
#include <utility>

using namespace std;

// ���� �Ŵ���
class GameManager
{
public:
	Game* GetGame(int roomIndex);
	void CreateGame(int roomIndex);
	bool RemoveGame(int roomIndex);

private:
	map<int, Game> mGames;	// ���� Map 
	/*
	* Map ���� ���� (unordered_map�� ����ص� �ǰڴµ�..?)
	* 1. ���Ի����� ����: vector X
	* 2. ã�Ⱑ �����ؾ��Ѵ�: list X
	* 3. Game �����θ� ã�� �� ����: Set X
	*/

	mutex mLock;			// ���ؽ� ��
};

