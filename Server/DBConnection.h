#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include <mysql.h>
#pragma comment(lib, "libmySQL.lib")

using namespace std;

/*
* DB 연결 클래스
*/
class DBConnection
{
public:
	DBConnection();
	~DBConnection();

	void SetTable(const char* tableName);
	int InsertData(map<string, string> data);
	vector<map<string, string>> SelectData(vector<string> columns, string whereStr = "", string orderStr = "", string limitStr = "");
	int UpdateData(unordered_map<string, string> updateData, string whereStr = "");
	int DeleteData(string whereStr = "");
	int Query(string query);

private:
	MYSQL mMysql;				// Mysql 구조체
	MYSQL* mConn = nullptr;		// Mysql 연결 포인터

	string mTableName = "";		// 테이블명

	// 웹서버에서 사용했던것처럼 .env와 같은 것이 있는지 알아봤으면 좋겠다.
	// 이것은 너무 위험하다.
	const char* mServer = "localhost";		// DB 서버
	const char* mUser = "test";				// DB 유저
	const char* mPW = "1111";				// DB 비밀번호
	const char* mDB = "cpp_test";			// DB 이름
	const int mPort = 3306;					// DB 포트
};