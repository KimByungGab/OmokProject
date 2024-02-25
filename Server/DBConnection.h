#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include <mysql.h>
#pragma comment(lib, "libmySQL.lib")

using namespace std;

class DBConnection
{
public:
	DBConnection();
	~DBConnection();

	void SetTable(const char* tableName) { mTableName = const_cast<char*>(tableName); }
	int InsertData(map<string, string> data);
	vector<map<string, string>> SelectData(vector<string> columns, string whereStr = "", string orderStr = "", string limitStr = "");
	int UpdateData(unordered_map<string, string> updateData, string whereStr = "");
	int DeleteData(string whereStr = "");
	int Query(string query);

private:
	MYSQL mMysql;
	MYSQL* mConn = nullptr;

	string mTableName = "";

	const char* mServer = "localhost";
	const char* mUser = "test";
	const char* mPW = "1111";
	const char* mDB = "cpp_test";
	const int mPort = 3306;
};