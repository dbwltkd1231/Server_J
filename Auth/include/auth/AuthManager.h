#pragma once
#include <string>

#include "../include/network/NetworkManager.h"
#include "../include/database/Worker.h"

namespace Auth
{
	class AuthManager
	{
	public:
		AuthManager();
		~AuthManager();

	public:
		void Initialize();
		void ConnectDatabase(std::string databaseName, std::string sqlServerAddress);

	public:
		void ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue);

	private:
		Network::NetworkManager _networkManager;

	private:
		SQLHENV _henv;
		SQLHDBC _hdbc;
		SQLHSTMT _hstmt;

	private:
		Database::Worker _userDatabaseWorker;
		Database::Worker _insertWorker;
		Database::Worker _deleteWorker;
		Database::Worker _updateWorker;


	private:
		void DatabaseCallback(ULONG_PTR& targetSocket, uint32_t& contentsType, SQLHSTMT& hstmt);

	};
}