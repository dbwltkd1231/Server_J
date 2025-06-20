#pragma once

//#define SettingMode  

#include "../Auth/AuthManager.h"
#include "../utility/ConfigCreator.h"

#include "../include/utility/ConstValue.h"
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

	int networkPort = config["NETWORK"]["PORT"].get<int>();
	int socketOnetimePrepareCount = config["NETWORK"]["SOCKET_ONETIME_PREPARE_COUNT"].get<int>();
	int clientActivateCountMax = config["NETWORK"]["CLIENT_ACTIVATE_COUNT_MAX"].get<int>();
	int clientAcceptReadyCountMax = config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"].get<int>();
	int overlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"].get<int>();
	int bufferSizeMax = config["NETWORK"]["BUFFER_SIZE_MAX"].get<int>();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int sessionCount = sysInfo.dwNumberOfProcessors * 2;


	Utility::ConstValue::GetInstance().ServerPort = networkPort;
	Utility::ConstValue::GetInstance().BuferSizeMax = bufferSizeMax;
	Utility::ConstValue::GetInstance().PreparedSocketCountMax = socketOnetimePrepareCount;
	Utility::ConstValue::GetInstance().ConnectedClientCountMax = clientActivateCountMax;
	Utility::ConstValue::GetInstance().ConnectReadyClientCountMax = clientAcceptReadyCountMax;
	Utility::ConstValue::GetInstance().OverlappedCountMax = overlappedCountMax;
	Utility::ConstValue::GetInstance().SessionCountMax = sessionCount;


	std::string databaseName = config["SQL"]["USER_DB_NAME"];
	std::string sqlServerAddress = config["SQL"]["USER_DB_ADDRESS"];

	Auth::AuthManager authManager;
	authManager.Initialize();
	authManager.ConnectDatabase(databaseName, sqlServerAddress);


	while (true)
	{

	}

#endif

	return 0;
}