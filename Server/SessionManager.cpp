#include "SessionManager.h"

/*
* @brief 세션 매니저 초기화
* @author 김병갑
* @param maxSessionCount: 추가될 세션 개수
* @return void
*/
void SessionManager::Init(const int maxSessionCount)
{
	AddSession(maxSessionCount);
}

/*
* @brief 세션 인덱스 호출
* @author 김병갑
* @param clientIndex: 클라이언트 인덱스
* @return 세션 포인터
*/
Session* SessionManager::GetUserByConnIdx(int clientIndex)
{
	return mSessions[clientIndex];
}

/*
* @brief 세션 추가
* @author 김병갑
* @param sessionCount: 추가할 세션 갯수
* @return void
*/
void SessionManager::AddSession(const int sessionCount)
{
	// 현재 세션 개수
	int currentCount = mSessions.size();

	// 추가된 이후 총 세션의 개수
	int maxCount = currentCount + sessionCount;
	
	// 세션 추가
	for (int i = currentCount; i < maxCount; i++)
	{
		mSessions.push_back(new Session());
		mSessions[i]->Init(i, "");
	}
}

/*
* @brief 로비에 있는 세션들 호출
* @author 김병갑
* @return 세션 포인터 vector
*/
vector<Session*> SessionManager::GetLobbySessions()
{
	// 좋지 못한 코드인 것 같다.
	// 이렇게 짤 바엔 차라리 로비에 관한 컨테이너를 만들어서 따로 관리하는 것이 좋아보인다.

	vector<Session*> retVal;

	// 세션 전부 조사
	for (Session* pSession : mSessions)
	{
		// 로비에 있는 사람들만 추가
		if (pSession->GetCurrentLocation() == Session::LOCATION::LOBBY)
		{
			retVal.push_back(pSession);
		}
	}

	return retVal;
}
