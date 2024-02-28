#include "RoomManager.h"

/*
* @brief ��� �� ����Ʈ ���
* @author �躴��
* @return �� ����ü ������ ����Ʈ
*/
list<st_room*> RoomManager::GetRooms()
{
	list<st_room*> rooms;

	for (auto iter = mRooms.begin(); iter != mRooms.end(); ++iter)
	{
		rooms.push_back(&(*iter));
	}

	return rooms;
}

/*
* @brief �� ����
* @author �躴��
* @param roomName: �� �̸�
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::CreateRoom(WCHAR* roomName)
{
	// ��ȯ��
	COMMON_RESULT_DTO result;

	// ���� ũ�Ⱑ �ִ� ũ�Ⱑ �Ǿ��ٸ�
	if (mRooms.size() == MAX_ROOM_COUNT)
	{
		result.code = ERROR_CODE::TOO_MANY_ROOM;

		return result;
	}

	// �� ��ȣ�� �������� �ϱ� ���� �õ尪 ���ϱ�
	srand(static_cast<unsigned int>(time(NULL)));

	// �� ����
	st_room newRoomInfo;
	memset(newRoomInfo.roomName, 0, sizeof(newRoomInfo.roomName));

	// �� ��ȣ ����ȭ
	newRoomInfo.roomNumber = rand();
	memcpy(newRoomInfo.roomName, roomName, wcslen(roomName) * 2);

	{
		lock_guard<mutex> guard(mLock);

		mRooms.push_back(newRoomInfo);
	}

	result.code = ERROR_CODE::NOTHING;

	// �κ񿡼� ���� �� ����
	lobby_room_info roomInfo;
	roomInfo.roomIndex = newRoomInfo.roomNumber;
	memcpy(roomInfo.roomName, roomName, wcslen(roomName) * 2);
	roomInfo.roomNameSize = wcslen(roomName);
	roomInfo.currentUserCount = 0;
	roomInfo.totalUserCount = ROOM_MAX_MEMBER_COUNT;

	result.pObject = reinterpret_cast<void*>(&roomInfo);

	return result;
}

/*
* @brief ���� ���� list �ε���
* @author �躴��
* @param roomIndex: ã����� �� ��ȣ
* @return ���� list �ε���
*/
int RoomManager::GetListIndex(int roomIndex)
{
	int i = 0;

	// ���������� �ϳ��� ã�ƺ���
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

/*
* @brief �� ����
* @author �躴��
* @param sessionIndex: ���� �ε���
* @param roomIndex: �� ��ȣ
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::EnterRoom(int sessionIndex, int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	// �� ã��
	auto iter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// ���� �� ã�Ҵٸ�
	if (iter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	// �� ����� �� á�ٸ�
	if (iter->roomMembers.size() == ROOM_MAX_MEMBER_COUNT)
	{
		retVal.code = ERROR_CODE::TOO_MANY_MEMBER;

		return retVal;
	}

	// �� ����� �߰�
	{
		lock_guard<mutex> guard(mLock);

		iter->roomMembers.push_back(sessionIndex);
	}

	retVal.code = ERROR_CODE::NOTHING;

	return retVal;
}

/*
* @brief �� ����
* @author �躴��
* @param sessionIndex: ���� �ε���
* @param roomIndex: �� ��ȣ
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::LeaveRoom(int sessionIndex, int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	// �� ã��
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// ���� �� ã�Ҵٸ�
	if (roomIter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	// �� ��� �� ���� ã��
	auto memberIter = find_if(roomIter->roomMembers.begin(), roomIter->roomMembers.end(), [=](int& currentSessionIndex)
		{
			return sessionIndex == currentSessionIndex;
		});

	// ������ �� ã�Ҵٸ�
	if (memberIter == roomIter->roomMembers.end())
	{
		retVal.code = ERROR_CODE::NOT_IN_THIS_ROOM;

		return retVal;
	}

	// ������ �� ��� ����Ʈ���� ����
	{
		lock_guard<mutex> guard(mLock);

		roomIter->roomMembers.erase(memberIter);
	}
	
	// �濡 �ƹ��� ���ٸ�
	if (roomIter->roomMembers.empty())
	{
		retVal.pObject = const_cast<char*>(EMPTY);
	}

	// ������� �ʴٸ�
	else
	{
		retVal.pObject = const_cast<char*>("");
	}

	retVal.code = ERROR_CODE::NOTHING;

	return retVal;
}

/*
* @brief �� ����
* @author �躴��
* @param roomIndex: �� ��ȣ
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::DeleteRoom(int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	// �� ���� ����Ʈ �ε��� ã��
	int currentListIndex = GetListIndex(roomIndex);

	// �� ã�Ҵٸ�
	if (currentListIndex == -1)
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<void*>(currentListIndex);

	// �� ��ȣ�� ���� �� ã��
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// ���� �� ã�Ҵٸ�
	if (roomIter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	// ���� ����Ʈ���� ����
	lock_guard<mutex> guard(mLock);

	mRooms.erase(roomIter);

	return retVal;
}

/*
* @brief �� �ȿ� �ִ� ��� ����Ʈ ȣ��
* @author �躴��
* @param roonIndex: �� ��ȣ
* @return ��� ���� �ε��� ����Ʈ
*/
list<int> RoomManager::GetMembersInRoom(int roomIndex)
{
	list<int> retVal;

	// �� ��ȣ�� �� ã��
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// �� ã�Ҵٸ�
	if (roomIter == mRooms.end())
	{
		return retVal;
	}

	return roomIter->roomMembers;
}

/*
* @brief ���������� ���� ȣ��
* @author �躴��
* @param roomIndex: �� ��ȣ
* @return ������ ����. true�� ������, false�� �������� �ƴ�.
*/
bool RoomManager::GetPlayStatus(int roomIndex)
{
	// �� ��ȣ�� �� ã��
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});
	
	// �� ã�Ҵٸ�
	if (roomIter == mRooms.end())
		return false;

	return roomIter->isPlaying;
}

/*
* @brief ������ ���� ����
* @author �躴��
* @param roomIndex: �� ��ȣ
* @param isPlaying: ������ ����. true�� ������, false�� �������� �ƴ�
* @return void
*/
void RoomManager::SetPlayStatus(int roomIndex, bool isPlaying)
{
	// �� ��ȣ�� �� ã��
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// �� ã�Ҵٸ�
	if (roomIter == mRooms.end())
		return;

	lock_guard<mutex> guard(mLock);
	roomIter->isPlaying = isPlaying;
}
