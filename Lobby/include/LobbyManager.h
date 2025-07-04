#pragma once
#include <functional>
#include <vector>
#include <random>
#include "hiredis/hiredis.h"
#include "../network/NetworkManager.h"
#include "../lobby/DatabaseProtocol.h"
#include "../database/Worker.h"
#include "EventWorker.h"
#include "../utility/LockFreeCircleQueue.h"
#include "oneapi/tbb/concurrent_set.h"
#include "oneapi/tbb/concurrent_queue.h"


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
		bool verifyJWT(const std::string& token, const std::string& secret);
	private:
		Network::NetworkManager _networkManager;
		bool _serverOn;

	private:
		Database::Worker _userDatabaseWorker;
		Database::Worker _gameDatabaseWorker;
		redisContext* _redis;

	private:
		void ProcessAccept(ULONG_PTR& targetSocket);
		void ProcessDisconnect(ULONG_PTR& targetSocket, int errorCode);
		void ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue);
		void ProcessQueryResult(ULONG_PTR targetSocket, Database::DatabaseQueryType queryType, uint32_t contentsType, SQLHSTMT& hstmt);

	private:
		std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT&)> _callbackProcedureResult;

	private:
		tbb::concurrent_set<ULONG_PTR> _notLoginSocketSet;
		tbb::concurrent_map<ULONG_PTR, uint64_t> _socketLoginAccountMap;

	private:
		tbb::concurrent_queue<Lobby::EventWorker> _eventQueue;

	private:
		std::vector<Common::Protocol::GameItem> _gameItemVector;

	private:
		std::random_device _randomDevice;
		std::mt19937 _gen;

	private:
		void EventThread();
		void EventProcess(ULONG_PTR& targetSocket, Lobby::EventType eventProtocol);

	public:
		void MainProcess();

	};
}