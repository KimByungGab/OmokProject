#include "SessionManager.h"

void SessionManager::Init(const int maxSessionCount)
{
	AddSession(maxSessionCount);
}

Session* SessionManager::GetUserByConnIdx(int clientIndex)
{
	return mSessions[clientIndex];
}

void SessionManager::AddSession(const int sessionCount)
{
	int currentCount = mSessions.size();
	int maxCount = currentCount + sessionCount;
	
	for (int i = currentCount; i < maxCount; i++)
	{
		mSessions.push_back(new Session());
		mSessions[i]->Init(i, "");
	}
}

vector<Session*> SessionManager::GetLobbySessions()
{
	vector<Session*> retVal;

	for (Session* pSession : mSessions)
	{
		if (pSession->GetCurrentLocation() == Session::LOCATION::LOBBY)
		{
			retVal.push_back(pSession);
		}
	}

	return retVal;
}
