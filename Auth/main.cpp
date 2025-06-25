#pragma once

//#define SettingMode  

#include "auth/AuthManager.h"
#include "auth/ConstValue.h"

#include "../utility/ConfigCreator.h"

int main()
{
#if defined(SettingMode)
	Utility::CreateAuthServerSettingFiles();
#else

	auto config = Utility::LoadSettingFiles();
	if (config == NULL)
	{
		return 0;
	}

	std::string serverIP = config["NETWORK"]["IP"];
	int networkPort = config["NETWORK"]["AUTHPORT"].get<int>();
	int clientAcceptReadyCountMax = config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"].get<int>();
	int overlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"].get<int>();
	int redisPort = config["REDIS"]["PORT"];

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int sessionCount = sysInfo.dwNumberOfProcessors * 2;

	Auth::ConstValue::GetInstance().ServerPort = networkPort;
	Auth::ConstValue::GetInstance().RedisPort = redisPort;
	Auth::ConstValue::GetInstance().ConnectReadyClientCountMax = clientAcceptReadyCountMax;
	Auth::ConstValue::GetInstance().OverlappedCountMax = overlappedCountMax;
	Auth::ConstValue::GetInstance().SessionCountMax = sessionCount;

	std::string databaseName = config["SQL"]["USER_DB_NAME"];
	std::string sqlServerAddress = config["SQL"]["USER_DB_ADDRESS"];

	Auth::AuthManager authManager;
	authManager.Initialize();
	authManager.ConnectDatabase(databaseName, sqlServerAddress);
	authManager.ConnectRedis(serverIP, Auth::ConstValue::GetInstance().RedisPort);


	while (true)
	{

	}

#endif

	return 0;
}