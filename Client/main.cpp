#pragma once
#include <string>

#include "../Common/include/utility/ConfigCreator.h"

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

	Game::ConstValue::GetInstance().IP = config["NETWORK"]["IP"].get<std::string>();
	Game::ConstValue::GetInstance().AuthServerPort = config["NETWORK"]["AUTHPORT"];
	Game::ConstValue::GetInstance().OverlappedCountMax = config["NETWORK"]["OVERLAPPED_COUNT_MAX"];
	Game::ConstValue::GetInstance().TestUID = config["CLIENT"]["CLIENT_TEST_UID"].get<std::string>();
	Game::ConstValue::GetInstance().TestClientCount = config["CLIENT"]["TEST_CLIENT_COUNT"];
	Game::ConstValue::GetInstance().ThreadCount = config["CLIENT"]["TEST_THREAD_COUNT"];
	Game::ConstValue::GetInstance().CurrentClinetIndex.store(0, std::memory_order_release);

	Game::GameManager gameManager;
	gameManager.Initialize(Game::ConstValue::GetInstance().IP, Game::ConstValue::GetInstance().AuthServerPort, Game::ConstValue::GetInstance().TestClientCount);
	gameManager.Process(Game::ConstValue::GetInstance().ThreadCount);

	while (true)
	{

	}

#endif

	return 0;
}