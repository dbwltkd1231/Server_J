#pragma once

#include "../include/database/Worker.h"
#include "../include/utility/Debug.h"
#include "../include/utility/StringConverter.h"

namespace Database
{
	Worker::Worker()
	{
		_henv = SQL_NULL_HENV;
		_hdbc = SQL_NULL_HDBC;
		_hstmt = SQL_NULL_HSTMT;

		_isOn = false;
	}
	Worker::~Worker()
	{
		_isOn = false;

		if (_hstmt != SQL_NULL_HSTMT)
			SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
		if (_hdbc != SQL_NULL_HDBC)
			SQLFreeHandle(SQL_HANDLE_DBC, _hdbc);
		if (_henv != SQL_NULL_HENV)
			SQLFreeHandle(SQL_HANDLE_ENV, _henv);
	}

	void Worker::Initialize(std::string databaseName, std::string sqlServerAddress, std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT&)> procedureCallback)
	{
		_procedureCallback = procedureCallback;

		SQLWCHAR sqlState[6], message[256];
		SQLINTEGER nativeError;
		SQLSMALLINT messageLength;

		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_henv);
		//ODBC 버전 설정
		ret = SQLSetEnvAttr(_henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			SQLGetDiagRecW(SQL_HANDLE_DBC, _hdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
			std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
			return;
		}
		Utility::Log("Database", "Worker", databaseName + " MSSQL Version Setting Success");

		//연결 핸들 생성
		ret = SQLAllocHandle(SQL_HANDLE_DBC, _henv, &_hdbc);

		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			SQLGetDiagRecW(SQL_HANDLE_DBC, _hdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);
			std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
			return;
		}
		Utility::Log("Database", "Worker", databaseName + " MSSQL Handle Create Success");


		std::wstring wServerAddress(sqlServerAddress.begin(), sqlServerAddress.end());
		SQLWCHAR* connStr = const_cast<SQLWCHAR*>(wServerAddress.c_str());


		//DB에 연결
		//->UNICODE 버전
		ret = SQLDriverConnectW(_hdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			SQLGetDiagRecW(SQL_HANDLE_DBC, _hdbc, 1, sqlState, &nativeError, message, sizeof(message), &messageLength);

			std::wcout << L"ODBC 오류 발생: " << message << L" (SQLState: " << sqlState << L")\n";
			return;
		}
		Utility::Log("Database", "Worker", databaseName + " MSSQL DB Connect Success");

		ret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &_hstmt);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			Utility::LogError("Database", "Worker", databaseName + " ODBC 오류 발생");
			return;
		}
		Utility::Log("Database", "Worker", databaseName + " MSSQL Querry Handle Create Success");

		_taskQueue.Construct(1024);//1024 : 임시.
	}

	void Worker::Activate(bool isOn)
	{
		_isOn = isOn;

		if (_isOn)
		{
			std::thread thread([this]() { this->Process(); });
			thread.detach();
			Utility::Log("Database", "Worker", "Worker Thread Activated");
		}
		else
		{
			Utility::Log("Database", "Worker", "Worker Thread Deactivated");
		}
	}

	void Worker::Enqueue(Database::Task task)
	{
		_taskQueue.push(std::move(task));
	}

	void Worker::Process()
	{
		Database::Task task;

		while (_isOn)
		{
			if (_taskQueue.size() > 0)
			{
				task = _taskQueue.pop();

				auto socketPtr = task.SocketPtr;
				auto procedureName = task.ProcedureName;
				auto queryType = task.QueryType;
				auto contentsType = task.NetworkType;
				auto params = task.Parameters;
				ExecuteStoredProcedure(procedureName, params, socketPtr, queryType, contentsType);
			}
		}
	}

	void Worker::ExecuteStoredProcedure(const std::string& procedureName, const std::string& params, ULONG_PTR socketPtr, DatabaseQueryType QueryType, int contentsType)
	{
		SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &_hstmt);

		// SQL 문 준비
		std::string stringQuerry = "EXEC " + procedureName + params;
		std::wstring wstringquery = Utility::StringConverter::ConvertToSQLWCHAR(stringQuerry);
		//std::string query = "EXEC " + procedureName + " " + params;


		// SQL 문 실행
		SQLWCHAR* dataQuery = (SQLWCHAR*)wstringquery.c_str();
		SQLRETURN dataRet = SQLExecDirectW(_hstmt, dataQuery, SQL_NTS);


		if (dataRet == SQL_SUCCESS || dataRet == SQL_SUCCESS_WITH_INFO)
		{
			std::string log = "프로시저 실행 성공: " + procedureName;
			Utility::Log("Database", "Worker", log);

			Utility::Log("SQL DEBUG", "Query", stringQuerry);  // 최종 EXEC문 로그 출력
			_procedureCallback(socketPtr, QueryType, contentsType, _hstmt);
		}
		else 
		{
			std::string log = "프로시저 실행 실패: " + procedureName;
			Utility::LogError("Database", "Worker", log);
		}

		SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
	}
}