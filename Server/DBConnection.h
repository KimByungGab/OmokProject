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
* DB ���� Ŭ����
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
	MYSQL mMysql;				// Mysql ����ü
	MYSQL* mConn = nullptr;		// Mysql ���� ������

	string mTableName = "";		// ���̺��

	// ���������� ����ߴ���ó�� .env�� ���� ���� �ִ��� �˾ƺ����� ���ڴ�.
	// �̰��� �ʹ� �����ϴ�.
	const char* mServer = "localhost";		// DB ����
	const char* mUser = "test";				// DB ����
	const char* mPW = "1111";				// DB ��й�ȣ
	const char* mDB = "cpp_test";			// DB �̸�
	const int mPort = 3306;					// DB ��Ʈ
};