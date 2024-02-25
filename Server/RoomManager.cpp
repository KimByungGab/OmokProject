#include "RoomManager.h"

list<st_room*> RoomManager::GetRooms()
{
	list<st_room*> rooms;

	for (auto iter = mRooms.begin(); iter != mRooms.end(); ++iter)
	{
		rooms.push_back(&(*iter));
	}

	return rooms;
}

COMMON_RESULT_DTO RoomManager::CreateRoom(WCHAR* roomName)
{
	COMMON_RESULT_DTO result;
	if (mRooms.size() == MAX_ROOM_COUNT)
	{
		result.code = ERROR_CODE::TOO_MANY_ROOM;

		return result;
	}

	srand(static_cast<unsigned int>(time(NULL)));

	st_room newRoomInfo;
	memset(newRoomInfo.roomName, 0, sizeof(newRoomInfo.roomName));

	newRoomInfo.roomNumber = rand();
	memcpy(newRoomInfo.roomName, roomName, wcslen(roomName) * 2);

	lock_guard<mutex> guard(mLock);

	mRooms.push_back(newRoomInfo);

	result.code = ERROR_CODE::NOTHING;

	lobby_room_info roomInfo;
	roomInfo.roomIndex = newRoomInfo.roomNumber;
	memcpy(roomInfo.roomName, roomName, wcslen(roomName) * 2);
	roomInfo.roomNameSize = wcslen(roomName);
	roomInfo.currentUserCount = 0;
	roomInfo.totalUserCount = ROOM_MAX_MEMBER_COUNT;

	result.pObject = reinterpret_cast<char*>(&roomInfo);

	return result;
}

int RoomManager::GetListIndex(int roomIndex)
{
	int i = 0;

	lock_guard<mutex> guard(mLock);
	for (auto room : mRooms)
	{
		if (room.roomNumber == roomIndex)
			break;

		i++;
	}

	if (i == mRooms.size())
		return -1;

	return i;
}

COMMON_RESULT_DTO RoomManager::EnterRoom(int sessionIndex, int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	auto iter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	if (iter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	if (iter->roomMembers.size() == ROOM_MAX_MEMBER_COUNT)
	{
		retVal.code = ERROR_CODE::TOO_MANY_MEMBER;

		return retVal;
	}

	lock_guard<mutex> guard(mLock);

	iter->roomMembers.push_back(sessionIndex);

	retVal.code = ERROR_CODE::NOTHING;

	return retVal;
}

COMMON_RESULT_DTO RoomManager::LeaveRoom(int sessionIndex, int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	if (roomIter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	auto memberIter = find_if(roomIter->roomMembers.begin(), roomIter->roomMembers.end(), [=](int& currentSessionIndex)
		{
			return sessionIndex == currentSessionIndex;
		});

	if (memberIter == roomIter->roomMembers.end())
	{
		retVal.code = ERROR_CODE::NOT_IN_THIS_ROOM;

		return retVal;
	}

	lock_guard<mutex> guard(mLock);

	roomIter->roomMembers.erase(memberIter);
	
	if (roomIter->roomMembers.empty())
	{
		retVal.pObject = const_cast<char*>(EMPTY);
	}
	else
	{
		retVal.pObject = const_cast<char*>("");
	}

	retVal.code = ERROR_CODE::NOTHING;

	return retVal;
}

COMMON_RESULT_DTO RoomManager::DeleteRoom(int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	int currentListIndex = GetListIndex(roomIndex);
	if (currentListIndex == -1)
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<char*>(currentListIndex);

	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	if (roomIter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	lock_guard<mutex> guard(mLock);

	mRooms.erase(roomIter);

	return retVal;
}

list<int> RoomManager::GetMembersInRoom(int roomIndex)
{
	list<int> retVal;

	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	if (roomIter == mRooms.end())
	{
		return retVal;
	}

	return roomIter->roomMembers;
}

bool RoomManager::GetPlayStatus(int roomIndex)
{
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});
	if (roomIter == mRooms.end())
		return false;

	return roomIter->isPlaying;
}

void RoomManager::SetPlayStatus(int roomIndex, bool isPlaying)
{
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});
	if (roomIter == mRooms.end())
		return;

	lock_guard<mutex> guard(mLock);
	roomIter->isPlaying = isPlaying;
}
