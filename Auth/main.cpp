#pragma once

//#define SettingMode  

#include "AuthManager.h"
#include "ConstValue.h"

#include "../utility/ConfigCreator.h"

int main()
{
#if defined(SettingMode)
	Utility::CreateAuthServerSettingFiles("auth_config.json");
#else

	auto config = Utility::LoadSettingFiles("auth_config.json");
	if (config == NULL)
	{
		return 0;
	}

	std::string secretKey = config["Auth"]["SECRET_KEY"];
	std::string serverIP = config["NETWORK"]["IP"];
	int networkPort = config["NETWORK"]["AUTHPORT"].get<int>();
	int clientAcceptReadyCountMax = config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"].get<int>();
	int overlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"].get<int>();
	int redisPort = config["REDIS"]["PORT"];

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int sessionCount = sysInfo.dwNumberOfProcessors * 2;

	Auth::ConstValue::GetInstance().SecretKey = secretKey;
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


	std::thread mainThread([&authManager]() { authManager.Process(); });

	mainThread.join();

#endif

	return 0;
}