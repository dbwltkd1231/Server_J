#pragma once

#include "../include/database/Worker.h"
#include "../include/utility/Debug.h"

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

	void Worker::Initialize(std::string databaseName, std::string sqlServerAddress)
	{
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

				auto procedureName = task.ProcedureName;
				auto contentsType = task.MessageType;
				auto params = task.Parameters;
				ExecuteStoredProcedure(procedureName, params, contentsType);
			}
		}
	}

	void Worker::ExecuteStoredProcedure(const std::string& procedureName, const std::string& params, int contentsType)
	{
		SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &_hstmt);

		// SQL 문 준비
		std::string query = "EXEC " + procedureName + " " + params;

		// SQL 문 실행
		SQLWCHAR* dataQuery = (SQLWCHAR*)query.c_str();
		SQLRETURN dataRet = SQLExecDirectW(_hstmt, dataQuery, SQL_NTS);


		if (dataRet == SQL_SUCCESS || dataRet == SQL_SUCCESS_WITH_INFO)
		{
			std::cout << "프로시저 실행 성공: " << procedureName << std::endl;


		}
		else 
		{
			std::cerr << "프로시저 실행 실패: " << procedureName << std::endl;
		}

		SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
	}
}