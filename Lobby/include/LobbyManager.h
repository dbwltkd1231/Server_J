#pragma once
#include <functional>

#include "hiredis/hiredis.h"

#include "../network/NetworkManager.h"
#include "../database/Worker.h"

namespace Lobby
{
	class LobbyManager
	{
	public:
		LobbyManager();
		~LobbyManager();

	public:
		void Initialize();
		void ConnectDatabase(std::string userDatabaseName, std::string userSqlServerAddress, std::string gameDatabaseName, std::string gameSqlServerAddress);
		void ConnectRedis(std::string ip, int redisPort);

	private:
		Network::NetworkManager _networkManager;

	private:
		Database::Worker _userDatabaseWorker;
		Database::Worker _gameDatabaseWorker;
		redisContext* _redis;

	private:
		void ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue);
		void SendQueryResult(ULONG_PTR targetSocket, uint32_t contentsType, SQLHSTMT& hstmt);

	private:
		std::function<void(ULONG_PTR, uint32_t, SQLHSTMT&)> _databaseCallback;
	};
}