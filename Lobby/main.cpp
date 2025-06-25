#pragma once
#include <string>

#include "../Common/include/utility/ConfigCreator.h"
#include "utility/ConstValue.h"

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

	int overlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"].get<int>();
	int buferSizeMax = config["NETWORK"]["BUFFER_SIZE_MAX"].get<int>();
	int connectReadyClientCountMax = config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"].get<int>();
	int clientCapacity = config["NETWORK"]["CLIENT_CAPACITY"].get<int>();

	int redisPort = config["REDIS"]["PORT"].get<int>();


	//SYSTEM_INFO sysInfo;
	//GetSystemInfo(&sysInfo);
	//int sessionCount = sysInfo.dwNumberOfProcessors * 2;

	Lobby::Utility::ConstValue::GetInstance().IP = serverIP;
	Lobby::Utility::ConstValue::GetInstance().StartPort = networkPort;
	Lobby::Utility::ConstValue::GetInstance().StartCount = startCount;
	Lobby::Utility::ConstValue::GetInstance().OverlappedCountMax = overlappedCountMax;
	Lobby::Utility::ConstValue::GetInstance().BuferSizeMax = buferSizeMax;
	Lobby::Utility::ConstValue::GetInstance().ClientCapacity = clientCapacity;
	Lobby::Utility::ConstValue::GetInstance().ConnectReadyClientCountMax = connectReadyClientCountMax;
	//Utility::ConstValue::GetInstance().SessionCountMax = sessionCount;

	std::string databaseName = config["SQL"]["USER_DB_NAME"];
	std::string sqlServerAddress = config["SQL"]["USER_DB_ADDRESS"];

	while (true)
	{

	}

#endif

	return 0;
}