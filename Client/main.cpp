#pragma once
#include <string>

#include "../Common/include/utility/ConfigCreator.h"
#include "../Client/include/utility/ConstValue.h"
#include "../Common/include/utility/ConstValue.h"

#include "client/ClientManager.h"

//#define SettingMode  

int main()
{

#if defined(SettingMode)
	Utility::CreateClientSettingFiles();
#else

	auto config = Utility::LoadSettingFiles();
	if (config == NULL)
	{
		return 0;
	}

	Utility::ConstValue::GetInstance().IP = config["NETWORK"]["IP"].get<std::string>();
	Utility::ConstValue::GetInstance().ServerPort = config["NETWORK"]["PORT"];
	Utility::ConstValue::GetInstance().BuferSizeMax = config["NETWORK"]["BUFFER_SIZE_MAX"];
	Utility::ConstValue::GetInstance().OverlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"];

	Client::ConstValue::GetInstance().TestUID = config["CLIENT"]["CLIENT_TEST_UID"].get<std::string>();
	Client::ConstValue::GetInstance().ClientCount = config["CLIENT"]["TEST_CLIENT_COUNT"];
	Client::ConstValue::GetInstance().ThreadCount = config["CLIENT"]["TEST_THREAD_COUNT"];


	Client::ClientManager clientManager;
	clientManager.Initialize(Utility::ConstValue::GetInstance().IP, Utility::ConstValue::GetInstance().ServerPort,
		Client::ConstValue::GetInstance().ClientCount, Client::ConstValue::GetInstance().ThreadCount);
		
	clientManager.Process(Client::ConstValue::GetInstance().ThreadCount);

	while (true)
	{

	}

#endif

	return 0;
}