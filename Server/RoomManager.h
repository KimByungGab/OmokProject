#pragma once

#include <list>
#include <map>
#include <mutex>
#include <ctime>
#include <vector>

#include "CommonDTO.h"
#include "LobbyDTO.h"

using namespace std;

#define ROOM_MAX_MEMBER_COUNT 2		// �濡 �� �� �ִ� �� �ο���
#define MAX_ROOM_COUNT 40			// �� ���� ���� ��

#define EMPTY "EMPTY"				// �濡 �ƹ��� ���� �� ������������ ���

// �� ���� ����ü
struct st_room
{
	int roomNumber;				// �� ��ȣ
	WCHAR roomName[45];			// �� �̸�
	list<int> roomMembers;		// �� �ȿ� �ִ� ���� ����Ʈ
	bool isPlaying = false;		// ���� ������ ����
};

// �� �Ŵ��� Ŭ����
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
	list<st_room> mRooms;	// �� ���� ����Ʈ
	mutex mLock;			// ���ؽ� ��
};

