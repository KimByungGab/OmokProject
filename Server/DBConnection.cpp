#include "DBConnection.h"

/*
* @brief DB ����
* @author �躴��
* @return void
*/
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

/*
* @brief ���̺� ����
* @author �躴��
* @param tableName: ���̺��
* @return void
*/
void DBConnection::SetTable(const char* tableName)
{
	mTableName = const_cast<char*>(tableName);
}

/*
* @brief ������ �߰�
* @author �躴��
* @param data: �߰��� ������(map<column ���ڿ�, value ���ڿ�>)
* @return ������ ó�� MySQL ��
*/
int DBConnection::InsertData(map<string, string> data)
{
	string columnStr = "";		// column ���� string
	string valueStr = "";		// value ���� string

	// �� �����͸��� escape ó���ϸ鼭 string �������
	for (auto iter : data)
	{
		string escapedString;
		mysql_real_escape_string(&mMysql, &escapedString[0], iter.second.c_str(), iter.second.size());

		columnStr += iter.first + ",";
		valueStr += "'" + string(&escapedString[0]) + "',";
	}

	// �������� �ִ� , ����
	columnStr.pop_back();
	valueStr.pop_back();

	// �ϼ��� insert SQL��
	string insertSQL = "INSERT INTO " + mTableName
		+ "(" + columnStr + ") VALUES (" + valueStr + ");";

	return Query(insertSQL);
}

/*
* @brief ������ ��ȸ
* @author �躴��
* @param columns: ��ȸ�� �÷� vector (vector<column ���ڿ�>)
* @param whereStr: where��
* @param orderStr: order��
* @param limitStr: limit��
* @return ��ȸ�� ������ �� (vector<map<column ���ڿ�, value ���ڿ�>>)
*/
vector<map<string, string>> DBConnection::SelectData(vector<string> columns, string whereStr, string orderStr, string limitStr)
{
	string selectSQL = "";						// select SQL ��
	string columnStr = "";						// column ���� string
	vector<map<string, string>> totalResult;	// select�� ���� data. map<column, value>�� ����

	// A,B,C,D �� ���� �����
	for (auto column : columns)
	{
		columnStr += column + ",";
	}
	columnStr.pop_back();

	selectSQL += "SELECT " + columnStr + " FROM " + mTableName;

	// Where ������ �߰�
	if (whereStr != "")
	{
		selectSQL += " WHERE " + whereStr;
	}

	// Order ������ �߰�
	if (orderStr != "")
	{
		selectSQL += " ORDER BY " + orderStr;
	}

	// Limit ������ �߰�
	if (limitStr != "")
	{
		selectSQL += " LIMIT " + limitStr;
	}

	int errNum = Query(selectSQL);

	// ���� Ȯ��
	if (errNum > 0)
	{
		cout << "mysql error code: " << errNum << endl;

		map<string, string> info;
		info.insert(make_pair("errorCode", to_string(errNum)));
		totalResult.push_back(info);

		return totalResult;
	}

	// ������ ã��
	MYSQL_RES* pResult = mysql_store_result(mConn);
	MYSQL_ROW row;

	// ������ ��ȸ�ϸ鼭 map�� <column, value> ���·� �߰�
	while ((row = mysql_fetch_row(pResult)) != NULL)
	{
		map<string, string> info;
		for (int i = 0; i < columns.size(); i++)
		{
			info.insert(make_pair(columns[i], row[i]));
		}
		totalResult.push_back(info);
	}

	// mysql ������ ����
	mysql_free_result(pResult);

	return totalResult;
}

/*
* @brief ������ ����
* @author �躴��
* @param updateData: ������ �÷� unordered_map (unordered_map<column ���ڿ�, value ���ڿ�>)
* @param whereStr: where��
* @return ������ ó�� MySQL ��
*/
int DBConnection::UpdateData(unordered_map<string, string> updateData, string whereStr)
{
	string updateSQL = "";		// Update ���� string
	string updateInfo = "";		// Update ��� string

	// <column, value> ������ unordered_map���� ������ ��ȸ
	// unordered map���� �� ����: �ߺ��Ǵ� Key�� ���� ������ �ؽ����̺��� ���¸� ����ִ� unordered map�� �����ҰŶ� �Ǵ�
	for (auto iter : updateData)
	{
		string escapedString;
		mysql_real_escape_string(&mMysql, &escapedString[0], iter.second.c_str(), iter.second.size());

		updateInfo += iter.first + "='" + &escapedString[0] + "',";
	}
	updateInfo.pop_back();

	// SQL�� �ϼ�
	updateSQL = "UPDATE " + mTableName + " SET " + updateInfo;

	// Where�� ������ �߰�
	if (whereStr != "")
	{
		updateSQL += " WHERE " + whereStr;
	}

	return Query(updateSQL);
}

/*
* @brief ������ ����
* @author �躴��
* @param whereStr: where��
* @return ������ ó�� MySQL ��
*/
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

/*
* @brief ���� ���� ��� �Լ�
* @author �躴��
* @param query: SQL Query
* @return ������ ó�� MySQL ��
*/
int DBConnection::Query(string query)
{
	return mysql_query(mConn, query.c_str());
}

// �Ҹ���
DBConnection::~DBConnection()
{
	mysql_close(mConn);
}
