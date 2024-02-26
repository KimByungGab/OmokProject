#include "Session.h"

/*
* @brief 세션 초기화
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @param id: 클라이언트 ID
* @return void
*/
void Session::Init(const int clientIndex, const char* id)
{
	// 인덱스와 id 입력
	mIndex = clientIndex;
	mID = string(id);
}

/*
* @brief 세션 클리어
* @author 김병갑
* @return void
*/
void Session::Clear()
{
	// 참가한 방 번호 초기화
	mRoomIndex = -1;

	// 현재 위치 초기화
	mCurrentLocation = LOCATION::NONE;

	// 패킷 버퍼 읽기,쓰기 포지션 초기화
	mPacketDataBufferWPos = 0;
	mPacketDataBufferRPos = 0;
}

/*
* @brief 로비 입장
* @author 김병갑
* @return void
*/
void Session::EnterLobby()
{
	// 방 인덱스는 초기화
	mRoomIndex = -1;

	// 현재 위치는 로비
	mCurrentLocation = LOCATION::LOBBY;
}

/*
* @brief 방 입장
* @author 김병갑
* @param roomIndex: 방 번호
* @return void
*/
void Session::EnterRoom(int roomIndex)
{
	// 방 번호 입력
	mRoomIndex = roomIndex;

	// 현재 위치는 방
	mCurrentLocation = LOCATION::ROOM;
}

/*
* @brief 현재 위치 호출
* @author 김병갑
* @return Session::Location enum
*/
Session::LOCATION Session::GetCurrentLocation()
{
	return mCurrentLocation;
}

/*
* @brief 현재 위치 세팅
* @author 김병갑
* @param location Session::Location enum
* @return void
*/
void Session::SetCurrentLocation(LOCATION location)
{
	mCurrentLocation = location;
}

/*
* @brief 현재 방 번호 호출
* @author 김병갑
* @return 방 번호
*/
int Session::GetCurrentRoom()
{
	return mRoomIndex;
}

/*
* @brief 연결된 클라이언트 인덱스 호출
* @author 김병갑
* @return 클라이언트 인덱스
*/
int Session::GetConnIdx()
{
	return mIndex;
}

/*
* @brief 아이디 세팅
* @author 김병갑
* @param id: 아이디
* @return void
*/
void Session::SetId(string id)
{
	mID = id;
}

/*
* @brief: 아이디 호출
* @author 김병갑
* @return 아이디
*/
string Session::GetId()
{
	return mID;
}

/*
* @brief DB 유저 idx 세팅
* @author 김병갑
* @param idx: DB 유저 idx
* @return void
*/
void Session::SetIdx(int idx)
{
	mIdx = idx;
}

/*
* @brief DB 유저 idx 호출
* @author 김병갑
* @return DB 유저 idx
*/
int Session::GetIdx()
{
	return mIdx;
}

/*
* @brief 패킷 가져오기
* @author 김병갑
* @return Packet 정보
*/
PacketInfo Session::GetPacket()
{
	// 남은 바이트. 버퍼에 쓴 곳부터 읽은 곳까지의 차이.
	int remainByte = mPacketDataBufferWPos - mPacketDataBufferRPos;

	// 남은 바이트가 패킷 헤더의 길이보다 짧다면. 즉, 패킷에 오류가 있다.
	// 최소 조건으로 패킷 헤더의 길이만큼 담아야되는데 그러질 못했다는것은 말이 안 됨.
	if (remainByte < PACKET_HEADER_LENGTH)
		return PacketInfo();

	// 버퍼의 읽은 곳에서 패킷 헤더 구조체로 형변환
	auto pHeader = (PACKET_HEADER*)&mPacketDataBuffer[mPacketDataBufferRPos];

	// 패킷의 길이가 남은 바이트보다 길다면.
	// 즉, 끊어졌다는 뜻이지만 패킷 데이터를 버퍼에 쓸 때 처리를 하기 때문에 이것도 말이 안 됨.
	if (pHeader->PacketLength > remainByte)
		return PacketInfo();

	// 패킷 정보
	PacketInfo packetInfo;
	packetInfo.PacketID = pHeader->PacketID;
	packetInfo.DataSize = pHeader->PacketLength;
	packetInfo.pDataPtr = &mPacketDataBuffer[mPacketDataBufferRPos];

	// 해당 패킷만큼 읽었기 때문에 읽은 위치를 패킷의 사이즈만큼 이동
	mPacketDataBufferRPos += pHeader->PacketLength;

	return packetInfo;
}

/*
* @brief 패킷 데이터를 버퍼에 쓰기
* @author 김병갑
* @param dataSize: 패킷 데이터 사이즈
* @param pData: 패킷 데이터
* @return void
*/
void Session::SetPacketData(const int dataSize, char* pData)
{
	// 패킷을 쓴 곳 + 데이터 사이즈 가 패킷의 범위를 넘어선다면
	if ((mPacketDataBufferWPos + dataSize) >= PACKET_DATA_BUFFER_SIZE)
	{
		// 패킷의 남은 사이즈를 계산
		auto remainDataSize = mPacketDataBufferWPos - mPacketDataBufferRPos;

		// 남은 데이터가 있다면
		if (remainDataSize > 0)
		{
			// 패킷의 맨 처음으로 남은 사이즈를 복사. 즉, 처음으로 남은 사이즈만큼 데이터를 옮겼다.
			memcpy(&mPacketDataBuffer[0], &mPacketDataBuffer[mPacketDataBufferRPos], remainDataSize);

			// 사이즈만큼 쓰기 위치 이동
			mPacketDataBufferWPos = remainDataSize;
		}

		// 남은 데이터가 없으면
		else
		{
			// 쓰기 위치도 처음 위치로 이동
			mPacketDataBufferWPos = 0;
		}

		// 읽기 위치를 처음 위치로 이동
		mPacketDataBufferRPos = 0;
	}

	// 패킷의 정보를 읽기 쓰기 위치에서 복사
	memcpy(&mPacketDataBuffer[mPacketDataBufferWPos], pData, dataSize);

	// 패킷의 위치를 데이터의 사이즈만큼 이동
	mPacketDataBufferWPos += dataSize;
}
