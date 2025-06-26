#pragma once
#include <string>
#include "LobbyManager.h"
#include "ConstValue.h"
#include "../Common/include/utility/ConfigCreator.h"


//#define SettingMode 

int main()
{
#if defined(SettingMode)
	Utility::CreateLobbySettingFiles();
#else

	auto config = Utility::LoadSettingFiles();
	if (config == NULL)
	{
		return 0;
	}

	std::string serverIP = config["NETWORK"]["IP"];
	int networkPort = config["NETWORK"]["Lobby_START_PORT"].get<int>();
	int startCount = config["NETWORK"]["Lobby_START_COUNT"].get<int>();
	int redisPort = config["REDIS"]["PORT"].get<int>();
	int overlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"].get<int>();
	int buferSizeMax = config["NETWORK"]["BUFFER_SIZE_MAX"].get<int>();
	int connectReadyClientCountMax = config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"].get<int>();
	int clientCapacity = config["NETWORK"]["CLIENT_CAPACITY"].get<int>();



	//SYSTEM_INFO sysInfo;
	//GetSystemInfo(&sysInfo);
	//int sessionCount = sysInfo.dwNumberOfProcessors * 2;

	Lobby::ConstValue::GetInstance().IP = serverIP;
	Lobby::ConstValue::GetInstance().StartPort = networkPort;
	Lobby::ConstValue::GetInstance().StartCount = startCount;
	Lobby::ConstValue::GetInstance().RedisPort = redisPort;
	Lobby::ConstValue::GetInstance().OverlappedCountMax = overlappedCountMax;
	Lobby::ConstValue::GetInstance().BuferSizeMax = buferSizeMax;
	Lobby::ConstValue::GetInstance().ClientCapacity = clientCapacity;
	Lobby::ConstValue::GetInstance().ConnectReadyClientCountMax = connectReadyClientCountMax;

	//Utility::ConstValue::GetInstance().SessionCountMax = sessionCount;

	std::string userDatabaseName = config["SQL"]["USER_DB_NAME"];
	std::string userDatabaseAddress = config["SQL"]["USER_DB_ADDRESS"];
	std::string gameDatabaseame = config["SQL"]["GAME_DB_NAME"];
	std::string gameDatabaseAddress = config["SQL"]["GAME_DB_ADDRESS"];
	Lobby::LobbyManager lobbyManager;
	lobbyManager.Initialize();
	lobbyManager.ConnectDatabase(userDatabaseName, userDatabaseAddress, gameDatabaseame, gameDatabaseAddress);
	lobbyManager.ConnectRedis(Lobby::ConstValue::GetInstance().IP, Lobby::ConstValue::GetInstance().RedisPort);

	while (true)
	{

	}

#endif

	return 0;
}