#pragma once
#include <string>
#include <functional>
#include <thread>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "../database/Task.h"
#include "../utility/LockFreeCircleQueue.h"

namespace Database
{
	class Worker
	{
	public:
		Worker();
		~Worker();

	private:
		SQLHENV _henv;
		SQLHDBC _hdbc;
		SQLHSTMT _hstmt;

	public:
		void Initialize(std::string databaseName, std::string sqlServerAddress, std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT&)> procedureCallback);
		void Activate(bool isOn);
		void Enqueue(Database::Task task);
	private:
		void Process();
		void ExecuteStoredProcedure(const std::string& procedureName, const std::string& params, ULONG_PTR socketPtr, DatabaseQueryType Querytype, int contentsType);
	private:
		bool _isOn;
	private:
		Utility::LockFreeCircleQueue<Database::Task> _taskQueue;
	private:
		std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT&)> _procedureCallback;

	};

}