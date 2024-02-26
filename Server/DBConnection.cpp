#include "DBConnection.h"

/*
* @brief DB 접속
* @author 김병갑
* @return void
*/
DBConnection::DBConnection()
{
	// MYSQL 구조체 초기화
	if (mysql_init(&mMysql) == nullptr)
	{
		cout << "mysql init error" << endl;
		return;
	}

	// DB 접속 후 연결 포인터객체 받기
	mConn = mysql_real_connect(&mMysql, mServer, mUser, mPW, mDB, mPort, nullptr, 0);
	if (mConn == nullptr)
	{
		cout << "mysql real connect error" << endl;
		return;
	}
}

/*
* @brief 테이블 세팅
* @author 김병갑
* @param tableName: 테이블명
* @return void
*/
void DBConnection::SetTable(const char* tableName)
{
	mTableName = const_cast<char*>(tableName);
}

/*
* @brief 데이터 추가
* @author 김병갑
* @param data: 추가할 데이터(map<column 문자열, value 문자열>)
* @return 데이터 처리 MySQL 값
*/
int DBConnection::InsertData(map<string, string> data)
{
	string columnStr = "";		// column 관련 string
	string valueStr = "";		// value 관련 string

	// 각 데이터마다 escape 처리하면서 string 만들어줌
	for (auto iter : data)
	{
		string escapedString;
		mysql_real_escape_string(&mMysql, &escapedString[0], iter.second.c_str(), iter.second.size());

		columnStr += iter.first + ",";
		valueStr += "'" + string(&escapedString[0]) + "',";
	}

	// 마지막에 있는 , 빼기
	columnStr.pop_back();
	valueStr.pop_back();

	// 완성된 insert SQL문
	string insertSQL = "INSERT INTO " + mTableName
		+ "(" + columnStr + ") VALUES (" + valueStr + ");";

	return Query(insertSQL);
}

/*
* @brief 데이터 조회
* @author 김병갑
* @param columns: 조회할 컬럼 vector (vector<column 문자열>)
* @param whereStr: where문
* @param orderStr: order문
* @param limitStr: limit문
* @return 조회한 데이터 값 (vector<map<column 문자열, value 문자열>>)
*/
vector<map<string, string>> DBConnection::SelectData(vector<string> columns, string whereStr, string orderStr, string limitStr)
{
	string selectSQL = "";						// select SQL 문
	string columnStr = "";						// column 관련 string
	vector<map<string, string>> totalResult;	// select로 나온 data. map<column, value>의 형태

	// A,B,C,D 의 형태 만들기
	for (auto column : columns)
	{
		columnStr += column + ",";
	}
	columnStr.pop_back();

	selectSQL += "SELECT " + columnStr + " FROM " + mTableName;

	// Where 있으면 추가
	if (whereStr != "")
	{
		selectSQL += " WHERE " + whereStr;
	}

	// Order 있으면 추가
	if (orderStr != "")
	{
		selectSQL += " ORDER BY " + orderStr;
	}

	// Limit 있으면 추가
	if (limitStr != "")
	{
		selectSQL += " LIMIT " + limitStr;
	}

	int errNum = Query(selectSQL);

	// 에러 확인
	if (errNum > 0)
	{
		cout << "mysql error code: " << errNum << endl;

		map<string, string> info;
		info.insert(make_pair("errorCode", to_string(errNum)));
		totalResult.push_back(info);

		return totalResult;
	}

	// 데이터 찾기
	MYSQL_RES* pResult = mysql_store_result(mConn);
	MYSQL_ROW row;

	// 데이터 순회하면서 map에 <column, value> 형태로 추가
	while ((row = mysql_fetch_row(pResult)) != NULL)
	{
		map<string, string> info;
		for (int i = 0; i < columns.size(); i++)
		{
			info.insert(make_pair(columns[i], row[i]));
		}
		totalResult.push_back(info);
	}

	// mysql 포인터 해제
	mysql_free_result(pResult);

	return totalResult;
}

/*
* @brief 데이터 갱신
* @author 김병갑
* @param updateData: 갱신할 컬럼 unordered_map (unordered_map<column 문자열, value 문자열>)
* @param whereStr: where문
* @return 데이터 처리 MySQL 값
*/
int DBConnection::UpdateData(unordered_map<string, string> updateData, string whereStr)
{
	string updateSQL = "";		// Update 관련 string
	string updateInfo = "";		// Update 대상 string

	// <column, value> 형태의 unordered_map으로 데이터 순회
	// unordered map으로 한 이유: 중복되는 Key가 없기 때문에 해시테이블의 형태를 띄고있는 unordered map이 유리할거라 판단
	for (auto iter : updateData)
	{
		string escapedString;
		mysql_real_escape_string(&mMysql, &escapedString[0], iter.second.c_str(), iter.second.size());

		updateInfo += iter.first + "='" + &escapedString[0] + "',";
	}
	updateInfo.pop_back();

	// SQL문 완성
	updateSQL = "UPDATE " + mTableName + " SET " + updateInfo;

	// Where절 있으면 추가
	if (whereStr != "")
	{
		updateSQL += " WHERE " + whereStr;
	}

	return Query(updateSQL);
}

/*
* @brief 데이터 삭제
* @author 김병갑
* @param whereStr: where문
* @return 데이터 처리 MySQL 값
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
* @brief 쿼리 직접 사용 함수
* @author 김병갑
* @param query: SQL Query
* @return 데이터 처리 MySQL 값
*/
int DBConnection::Query(string query)
{
	return mysql_query(mConn, query.c_str());
}

// 소멸자
DBConnection::~DBConnection()
{
	mysql_close(mConn);
}
