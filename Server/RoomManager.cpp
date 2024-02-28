#include "RoomManager.h"

/*
* @brief 모든 방 리스트 얻기
* @author 김병갑
* @return 방 구조체 포인터 리스트
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
* @brief 방 생성
* @author 김병갑
* @param roomName: 방 이름
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::CreateRoom(WCHAR* roomName)
{
	// 반환값
	COMMON_RESULT_DTO result;

	// 방의 크기가 최대 크기가 되었다면
	if (mRooms.size() == MAX_ROOM_COUNT)
	{
		result.code = ERROR_CODE::TOO_MANY_ROOM;

		return result;
	}

	// 방 번호를 랜덤으로 하기 위한 시드값 정하기
	srand(static_cast<unsigned int>(time(NULL)));

	// 방 정보
	st_room newRoomInfo;
	memset(newRoomInfo.roomName, 0, sizeof(newRoomInfo.roomName));

	// 방 번호 랜덤화
	newRoomInfo.roomNumber = rand();
	memcpy(newRoomInfo.roomName, roomName, wcslen(roomName) * 2);

	{
		lock_guard<mutex> guard(mLock);

		mRooms.push_back(newRoomInfo);
	}

	result.code = ERROR_CODE::NOTHING;

	// 로비에서 보는 방 정보
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
* @brief 방의 실제 list 인덱스
* @author 김병갑
* @param roomIndex: 찾고싶은 방 번호
* @return 실제 list 인덱스
*/
int RoomManager::GetListIndex(int roomIndex)
{
	int i = 0;

	// 순차적으로 하나씩 찾아보기
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
* @brief 방 입장
* @author 김병갑
* @param sessionIndex: 세션 인덱스
* @param roomIndex: 방 번호
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::EnterRoom(int sessionIndex, int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	// 방 찾기
	auto iter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// 방을 못 찾았다면
	if (iter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	// 방 멤버가 꽉 찼다면
	if (iter->roomMembers.size() == ROOM_MAX_MEMBER_COUNT)
	{
		retVal.code = ERROR_CODE::TOO_MANY_MEMBER;

		return retVal;
	}

	// 방 멤버에 추가
	{
		lock_guard<mutex> guard(mLock);

		iter->roomMembers.push_back(sessionIndex);
	}

	retVal.code = ERROR_CODE::NOTHING;

	return retVal;
}

/*
* @brief 방 퇴장
* @author 김병갑
* @param sessionIndex: 세션 인덱스
* @param roomIndex: 방 번호
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::LeaveRoom(int sessionIndex, int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	// 방 찾기
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// 방을 못 찾았다면
	if (roomIter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	// 방 멤버 중 본인 찾기
	auto memberIter = find_if(roomIter->roomMembers.begin(), roomIter->roomMembers.end(), [=](int& currentSessionIndex)
		{
			return sessionIndex == currentSessionIndex;
		});

	// 본인을 못 찾았다면
	if (memberIter == roomIter->roomMembers.end())
	{
		retVal.code = ERROR_CODE::NOT_IN_THIS_ROOM;

		return retVal;
	}

	// 본인을 방 멤버 리스트에서 삭제
	{
		lock_guard<mutex> guard(mLock);

		roomIter->roomMembers.erase(memberIter);
	}
	
	// 방에 아무도 없다면
	if (roomIter->roomMembers.empty())
	{
		retVal.pObject = const_cast<char*>(EMPTY);
	}

	// 비어있지 않다면
	else
	{
		retVal.pObject = const_cast<char*>("");
	}

	retVal.code = ERROR_CODE::NOTHING;

	return retVal;
}

/*
* @brief 방 삭제
* @author 김병갑
* @param roomIndex: 방 번호
* @return Common result DTO
*/
COMMON_RESULT_DTO RoomManager::DeleteRoom(int roomIndex)
{
	COMMON_RESULT_DTO retVal;

	// 방 실제 리스트 인덱스 찾기
	int currentListIndex = GetListIndex(roomIndex);

	// 못 찾았다면
	if (currentListIndex == -1)
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	retVal.code = ERROR_CODE::NOTHING;
	retVal.pObject = reinterpret_cast<void*>(currentListIndex);

	// 방 번호로 현재 방 찾기
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// 방을 못 찾았다면
	if (roomIter == mRooms.end())
	{
		retVal.code = ERROR_CODE::WRONG_ROOM;

		return retVal;
	}

	// 방을 리스트에서 삭제
	lock_guard<mutex> guard(mLock);

	mRooms.erase(roomIter);

	return retVal;
}

/*
* @brief 방 안에 있는 멤버 리스트 호출
* @author 김병갑
* @param roonIndex: 방 번호
* @return 멤버 세션 인덱스 리스트
*/
list<int> RoomManager::GetMembersInRoom(int roomIndex)
{
	list<int> retVal;

	// 방 번호로 방 찾기
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// 못 찾았다면
	if (roomIter == mRooms.end())
	{
		return retVal;
	}

	return roomIter->roomMembers;
}

/*
* @brief 게임중인지 상태 호출
* @author 김병갑
* @param roomIndex: 방 번호
* @return 게임중 여부. true면 게임중, false면 게임중이 아님.
*/
bool RoomManager::GetPlayStatus(int roomIndex)
{
	// 방 번호로 방 찾기
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});
	
	// 못 찾았다면
	if (roomIter == mRooms.end())
		return false;

	return roomIter->isPlaying;
}

/*
* @brief 게임중 여부 세팅
* @author 김병갑
* @param roomIndex: 방 번호
* @param isPlaying: 게임중 여부. true면 게임중, false면 게임중이 아님
* @return void
*/
void RoomManager::SetPlayStatus(int roomIndex, bool isPlaying)
{
	// 방 번호로 방 찾기
	auto roomIter = find_if(mRooms.begin(), mRooms.end(), [=](st_room& currentRoom)
		{
			return roomIndex == currentRoom.roomNumber;
		});

	// 못 찾았다면
	if (roomIter == mRooms.end())
		return;

	lock_guard<mutex> guard(mLock);
	roomIter->isPlaying = isPlaying;
}
