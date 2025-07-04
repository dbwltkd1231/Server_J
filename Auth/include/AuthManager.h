#pragma once
#include <string>
#include <functional>

#include "hiredis/hiredis.h"
#include "../include/network/NetworkManager.h"
#include "../include/database/Worker.h"

#include <openssl/hmac.h>

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
		void ConnectRedis(std::string ip, int redisPort);

	private:
		bool isOn;

	public:
		void Process();

	public:
		void ProcessAccept(ULONG_PTR& targetSocket);
		void ProcessDisconnect(ULONG_PTR& targetSocket, int errorCode);
		void ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue);

	private:
		Network::NetworkManager _networkManager;

	private:
		SQLHENV _henv;
		SQLHDBC _hdbc;
		SQLHSTMT _hstmt;

	private:
		Database::Worker _userDatabaseWorker;

	private:
		std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT&)> _databaseCallback;

	private:
		redisContext* _redis;

	private:
		void DatabaseCallback(ULONG_PTR targetSocket, uint32_t contentsType, SQLHSTMT& hstmt);

	private:
		int CheckLobbyServerState();

	private:
		std::string createJWT(const std::string& userId, const std::string& secret);

	};
}