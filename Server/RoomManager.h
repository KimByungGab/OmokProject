#pragma once

#include <list>
#include <map>
#include <mutex>
#include <ctime>
#include <vector>

#include "CommonDTO.h"
#include "LobbyDTO.h"

using namespace std;

#define ROOM_MAX_MEMBER_COUNT 2		// 방에 들어갈 수 있는 총 인원수
#define MAX_ROOM_COUNT 40			// 방 생성 가능 수

#define EMPTY "EMPTY"				// 방에 아무도 없을 때 구분짓기위한 상수

// 방 정보 구조체
struct st_room
{
	int roomNumber;				// 방 번호
	WCHAR roomName[45];			// 방 이름
	list<int> roomMembers;		// 방 안에 있는 세션 리스트
	bool isPlaying = false;		// 현재 게임중 여부
};

// 방 매니저 클래스
class RoomManager
{
public:
	RoomManager() = default;
	~RoomManager() = default;

	list<st_room*> GetRooms();
	COMMON_RESULT_DTO CreateRoom(WCHAR* roomName);
	COMMON_RESULT_DTO EnterRoom(int sessionIndex, int roomIndex);
	COMMON_RESULT_DTO LeaveRoom(int sessionIndex, int roomIndex);
	COMMON_RESULT_DTO DeleteRoom(int roomIndex);
	int GetListIndex(int roomIndex);
	list<int> GetMembersInRoom(int roomIndex);
	
	bool GetPlayStatus(int roomIndex);
	void SetPlayStatus(int roomIndex, bool isPlaying);

public:
	const int mMaxRoomMemberCount = ROOM_MAX_MEMBER_COUNT;
	const char* mEmptyString = EMPTY;

private:
	list<st_room> mRooms;	// 방 정보 리스트
	mutex mLock;			// 뮤텍스 락
};

