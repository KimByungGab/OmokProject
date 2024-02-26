#include "SessionManager.h"

/*
* @brief ���� �Ŵ��� �ʱ�ȭ
* @author �躴��
* @param maxSessionCount: �߰��� ���� ����
* @return void
*/
void SessionManager::Init(const int maxSessionCount)
{
	AddSession(maxSessionCount);
}

/*
* @brief ���� �ε��� ȣ��
* @author �躴��
* @param clientIndex: Ŭ���̾�Ʈ �ε���
* @return ���� ������
*/
Session* SessionManager::GetUserByConnIdx(int clientIndex)
{
	return mSessions[clientIndex];
}

/*
* @brief ���� �߰�
* @author �躴��
* @param sessionCount: �߰��� ���� ����
* @return void
*/
void SessionManager::AddSession(const int sessionCount)
{
	// ���� ���� ����
	int currentCount = mSessions.size();

	// �߰��� ���� �� ������ ����
	int maxCount = currentCount + sessionCount;
	
	// ���� �߰�
	for (int i = currentCount; i < maxCount; i++)
	{
		mSessions.push_back(new Session());
		mSessions[i]->Init(i, "");
	}
}

/*
* @brief �κ� �ִ� ���ǵ� ȣ��
* @author �躴��
* @return ���� ������ vector
*/
vector<Session*> SessionManager::GetLobbySessions()
{
	// ���� ���� �ڵ��� �� ����.
	// �̷��� © �ٿ� ���� �κ� ���� �����̳ʸ� ���� ���� �����ϴ� ���� ���ƺ��δ�.

	vector<Session*> retVal;

	// ���� ���� ����
	for (Session* pSession : mSessions)
	{
		// �κ� �ִ� ����鸸 �߰�
		if (pSession->GetCurrentLocation() == Session::LOCATION::LOBBY)
		{
			retVal.push_back(pSession);
		}
	}

	return retVal;
}
