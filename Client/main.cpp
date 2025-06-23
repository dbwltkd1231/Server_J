#pragma once
#include <string>

#include "../Common/include/utility/ConfigCreator.h"
#include "../Client/include/utility/ConstValue.h"
#include "../Common/include/utility/ConstValue.h"

#include "game/GameManager.h"

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

	ClientUtility::ConstValue::GetInstance().TestUID = config["CLIENT"]["CLIENT_TEST_UID"].get<std::string>();
	ClientUtility::ConstValue::GetInstance().ClientCount = config["CLIENT"]["TEST_CLIENT_COUNT"];
	ClientUtility::ConstValue::GetInstance().ThreadCount = config["CLIENT"]["TEST_THREAD_COUNT"];

	Game::GameManager gameManager;
	gameManager.Initialize(Utility::ConstValue::GetInstance().IP, Utility::ConstValue::GetInstance().ServerPort);
	gameManager.Process(ClientUtility::ConstValue::GetInstance().ThreadCount);

//	Network::NetworkManager authManager;
//	Network::NetworkManager lobbyManager;
//	Game::UserManager userManager;
//
//	authManager.Initialize(Utility::ConstValue::GetInstance().IP, Utility::ConstValue::GetInstance().ServerPort,
//		Client::ConstValue::GetInstance().ClientCount, Client::ConstValue::GetInstance().ThreadCount);
//		
//
//
//	std::function<void(ULONG_PTR)> acceptCallback = std::function<void(ULONG_PTR)>(
//		[&userManager](ULONG_PTR targetSocket)
//		{
//			userManager.AcceptCallback(targetSocket);
//		}
//	);
//
//	authManager.CallbackSetting(acceptCallback);


//clientManager.CallbackSetting(
//	std::function<void(ULONG_PTR)>(
//		[&clientManager](ULONG_PTR targetSocket)
//		{
//			clientManager.AcceptCallback(targetSocket);
//		}
//	)
//);


	while (true)
	{

	}

#endif

	return 0;
}