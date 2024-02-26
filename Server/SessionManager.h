#pragma once

#include <vector>

#include "ErrorCode.h"
#include "Session.h"

using namespace std;

// 세션 매니저 클래스
class SessionManager
{
public:
	SessionManager() = default;
	~SessionManager() = default;

	void Init(const int maxSessionCount);
	Session* GetUserByConnIdx(int clientIndex);
	void AddSession(const int sessionCount);
	vector<Session*> GetLobbySessions();

private:
	vector<Session*> mSessions;		// 세션 포인터 vector
};

