#include "Session.h"

/*
* @brief ���� �ʱ�ȭ
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @param id: Ŭ���̾�Ʈ ID
* @return void
*/
void Session::Init(const int clientIndex, const char* id)
{
	// �ε����� id �Է�
	mIndex = clientIndex;
	mID = string(id);
}

/*
* @brief ���� Ŭ����
* @author �躴��
* @return void
*/
void Session::Clear()
{
	// ������ �� ��ȣ �ʱ�ȭ
	mRoomIndex = -1;

	// ���� ��ġ �ʱ�ȭ
	mCurrentLocation = LOCATION::NONE;

	// ��Ŷ ���� �б�,���� ������ �ʱ�ȭ
	mPacketDataBufferWPos = 0;
	mPacketDataBufferRPos = 0;
}

/*
* @brief �κ� ����
* @author �躴��
* @return void
*/
void Session::EnterLobby()
{
	// �� �ε����� �ʱ�ȭ
	mRoomIndex = -1;

	// ���� ��ġ�� �κ�
	mCurrentLocation = LOCATION::LOBBY;
}

/*
* @brief �� ����
* @author �躴��
* @param roomIndex: �� ��ȣ
* @return void
*/
void Session::EnterRoom(int roomIndex)
{
	// �� ��ȣ �Է�
	mRoomIndex = roomIndex;

	// ���� ��ġ�� ��
	mCurrentLocation = LOCATION::ROOM;
}

/*
* @brief ���� ��ġ ȣ��
* @author �躴��
* @return Session::Location enum
*/
Session::LOCATION Session::GetCurrentLocation()
{
	return mCurrentLocation;
}

/*
* @brief ���� ��ġ ����
* @author �躴��
* @param location Session::Location enum
* @return void
*/
void Session::SetCurrentLocation(LOCATION location)
{
	mCurrentLocation = location;
}

/*
* @brief ���� �� ��ȣ ȣ��
* @author �躴��
* @return �� ��ȣ
*/
int Session::GetCurrentRoom()
{
	return mRoomIndex;
}

/*
* @brief ����� Ŭ���̾�Ʈ �ε��� ȣ��
* @author �躴��
* @return Ŭ���̾�Ʈ �ε���
*/
int Session::GetConnIdx()
{
	return mIndex;
}

/*
* @brief ���̵� ����
* @author �躴��
* @param id: ���̵�
* @return void
*/
void Session::SetId(string id)
{
	mID = id;
}

/*
* @brief: ���̵� ȣ��
* @author �躴��
* @return ���̵�
*/
string Session::GetId()
{
	return mID;
}

/*
* @brief DB ���� idx ����
* @author �躴��
* @param idx: DB ���� idx
* @return void
*/
void Session::SetIdx(int idx)
{
	mIdx = idx;
}

/*
* @brief DB ���� idx ȣ��
* @author �躴��
* @return DB ���� idx
*/
int Session::GetIdx()
{
	return mIdx;
}

/*
* @brief ��Ŷ ��������
* @author �躴��
* @return Packet ����
*/
PacketInfo Session::GetPacket()
{
	// ���� ����Ʈ. ���ۿ� �� ������ ���� �������� ����.
	int remainByte = mPacketDataBufferWPos - mPacketDataBufferRPos;

	// ���� ����Ʈ�� ��Ŷ ����� ���̺��� ª�ٸ�. ��, ��Ŷ�� ������ �ִ�.
	// �ּ� �������� ��Ŷ ����� ���̸�ŭ ��ƾߵǴµ� �׷��� ���ߴٴ°��� ���� �� ��.
	if (remainByte < PACKET_HEADER_LENGTH)
		return PacketInfo();

	// ������ ���� ������ ��Ŷ ��� ����ü�� ����ȯ
	auto pHeader = (PACKET_HEADER*)&mPacketDataBuffer[mPacketDataBufferRPos];

	// ��Ŷ�� ���̰� ���� ����Ʈ���� ��ٸ�.
	// ��, �������ٴ� �������� ��Ŷ �����͸� ���ۿ� �� �� ó���� �ϱ� ������ �̰͵� ���� �� ��.
	if (pHeader->PacketLength > remainByte)
		return PacketInfo();

	// ��Ŷ ����
	PacketInfo packetInfo;
	packetInfo.PacketID = pHeader->PacketID;
	packetInfo.DataSize = pHeader->PacketLength;
	packetInfo.pDataPtr = &mPacketDataBuffer[mPacketDataBufferRPos];

	// �ش� ��Ŷ��ŭ �о��� ������ ���� ��ġ�� ��Ŷ�� �����ŭ �̵�
	mPacketDataBufferRPos += pHeader->PacketLength;

	return packetInfo;
}

/*
* @brief ��Ŷ �����͸� ���ۿ� ����
* @author �躴��
* @param dataSize: ��Ŷ ������ ������
* @param pData: ��Ŷ ������
* @return void
*/
void Session::SetPacketData(const int dataSize, char* pData)
{
	// ��Ŷ�� �� �� + ������ ������ �� ��Ŷ�� ������ �Ѿ�ٸ�
	if ((mPacketDataBufferWPos + dataSize) >= PACKET_DATA_BUFFER_SIZE)
	{
		// ��Ŷ�� ���� ����� ���
		auto remainDataSize = mPacketDataBufferWPos - mPacketDataBufferRPos;

		// ���� �����Ͱ� �ִٸ�
		if (remainDataSize > 0)
		{
			// ��Ŷ�� �� ó������ ���� ����� ����. ��, ó������ ���� �����ŭ �����͸� �Ű��.
			memcpy(&mPacketDataBuffer[0], &mPacketDataBuffer[mPacketDataBufferRPos], remainDataSize);

			// �����ŭ ���� ��ġ �̵�
			mPacketDataBufferWPos = remainDataSize;
		}

		// ���� �����Ͱ� ������
		else
		{
			// ���� ��ġ�� ó�� ��ġ�� �̵�
			mPacketDataBufferWPos = 0;
		}

		// �б� ��ġ�� ó�� ��ġ�� �̵�
		mPacketDataBufferRPos = 0;
	}

	// ��Ŷ�� ������ �б� ���� ��ġ���� ����
	memcpy(&mPacketDataBuffer[mPacketDataBufferWPos], pData, dataSize);

	// ��Ŷ�� ��ġ�� �������� �����ŭ �̵�
	mPacketDataBufferWPos += dataSize;
}
