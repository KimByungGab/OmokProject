#include "DBConnection.h"

// DB ����
DBConnection::DBConnection()
{
	// MYSQL ����ü �ʱ�ȭ
	if (mysql_init(&mMysql) == nullptr)
	{
		cout << "mysql init error" << endl;
		return;
	}

	// DB ���� �� ���� �����Ͱ�ü �ޱ�
	mConn = mysql_real_connect(&mMysql, mServer, mUser, mPW, mDB, mPort, nullptr, 0);
	if (mConn == nullptr)
	{
		cout << "mysql real connect error" << endl;
		return;
	}
}

// insert
int DBConnection::InsertData(map<string, string> data)
{
	string columnStr = "";
	string valueStr = "";

	for (auto iter : data)
	{
		string escapedString;
		mysql_real_escape_string(&mMysql, &escapedString[0], iter.second.c_str(), iter.second.size());

		columnStr += iter.first + ",";
		valueStr += "'" + string(&escapedString[0]) + "',";
	}
	columnStr.pop_back();
	valueStr.pop_back();

	string insertSQL = "INSERT INTO " + mTableName
		+ "(" + columnStr + ") VALUES (" + valueStr + ");";

	return Query(insertSQL);
}

// select
vector<map<string, string>> DBConnection::SelectData(vector<string> columns, string whereStr, string orderStr, string limitStr)
{
	string selectSQL = "";
	string columnStr = "";
	vector<map<string, string>> totalResult;

	for (auto column : columns)
	{
		columnStr += column + ",";
	}
	columnStr.pop_back();

	selectSQL += "SELECT " + columnStr + " FROM " + mTableName;

	if (whereStr != "")
	{
		selectSQL += " WHERE " + whereStr;
	}

	if (orderStr != "")
	{
		selectSQL += " ORDER BY " + orderStr;
	}

	if (limitStr != "")
	{
		selectSQL += " LIMIT " + limitStr;
	}

	int errNum = Query(selectSQL);
	if (errNum > 0)
	{
		cout << "mysql error code: " << errNum << endl;

		map<string, string> info;
		info.insert(make_pair("errorCode", to_string(errNum)));
		totalResult.push_back(info);

		return totalResult;
	}

	MYSQL_RES* pResult = mysql_store_result(mConn);
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pResult)) != NULL)
	{
		map<string, string> info;
		for (int i = 0; i < columns.size(); i++)
		{
			info.insert(make_pair(columns[i], row[i]));
		}
		totalResult.push_back(info);
	}

	mysql_free_result(pResult);

	return totalResult;
}

// update
int DBConnection::UpdateData(unordered_map<string, string> updateData, string whereStr)
{
	string updateSQL = "";
	string updateInfo = "";

	for (auto iter : updateData)
	{
		string escapedString;
		mysql_real_escape_string(&mMysql, &escapedString[0], iter.second.c_str(), iter.second.size());

		updateInfo += iter.first + "='" + &escapedString[0] + "',";
	}
	updateInfo.pop_back();

	updateSQL = "UPDATE " + mTableName + " SET " + updateInfo;

	if (whereStr != "")
	{
		updateSQL += " WHERE " + whereStr;
	}

	return Query(updateSQL);
}

// delete
int DBConnection::DeleteData(string whereStr)
{
	string deleteSQL = "";

	deleteSQL = "DELETE FROM " + mTableName;
	if (whereStr != "")
	{
		deleteSQL += " WHERE " + whereStr;
	}

	return Query(deleteSQL);
}

// raw ������, ������ ������ ���� ���� ��뵵 �ϰԲ� publicȭ
int DBConnection::Query(string query)
{
	return mysql_query(mConn, query.c_str());
}

// �Ҹ���
DBConnection::~DBConnection()
{
	mysql_close(mConn);
}
