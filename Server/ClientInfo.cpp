#include "ClientInfo.h"

// ������
ClientInfo::ClientInfo()
{
	// �ʱ�ȭ
	Init();
}

/*
* @brief Ŭ���̾�Ʈ �ε��� ȣ��
* @author �躴��
* @return �ε���
*/
int ClientInfo::GetIndex()
{
	return mIndex;
}

/*
* @brief Ŭ���̾�Ʈ �ε��� ����
* @author �躴��
* @param index: Ŭ���̾�Ʈ �ε���
* @return void
*/
void ClientInfo::SetIndex(const int index)
{
	mIndex = index;
}

/*
* @brief Ŭ���̾�Ʈ ���� ����
* @author �躴��
* @return ���� ����. true�� ����, false�� ����ȵ�
*/
bool ClientInfo::IsConnected()
{
	return mSocket != INVALID_SOCKET;
}

/*
* @brief Ŭ���̾�Ʈ ���� ȣ��
* @author �躴��
* @return ����
*/
SOCKET ClientInfo::GetSocket()
{
	return mSocket;
}

/*
* @brief Ŭ���̾�Ʈ ���� ����
* @author �躴��
* @param socket: ����
* @return void
*/
void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

/*
* @brief ���� ���� ȣ��
* @author �躴��
* @return ���� ���� ������
*/
char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}

/*
* @brief �۽� ���� ȣ��
* @author �躴��
* @return �۽� ���� ������
*/
char* ClientInfo::GetSendBuffer()
{
	return mSendBuf;
}

/*
* @brief Ŭ���̾�Ʈ ���� Ŭ����
* @author �躴��
* @return void
*/
void ClientInfo::Clear()
{
	Init();
}

/*
* @brief Ŭ���̾�Ʈ �ʱ�ȭ
* @author �躴��
* @return void
*/
void ClientInfo::Init()
{
	// ���� �� �۽� Overlapped Ȯ�� ����ü �ʱ�ȭ
	memset(&m_stRecvOverlapeedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));

	// ���� �ʱ�ȭ
	mSocket = INVALID_SOCKET;

	// ���� �� �۽� ���� �ʱ�ȭ
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
}
