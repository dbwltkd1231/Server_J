#pragma once
#include "Game/GameManager.h"

namespace Game
{
	GameManager::GameManager()
	{
		Utility::Log("Game", "GameManager", "Construct");
	}

	GameManager::~GameManager()
	{
		Utility::Log("Game", "GameManager", "Destruct");
	}

	void GameManager::Initialize(std::string ip, int port)
	{
		auto client = std::make_shared<Network::Client>();
		_networkManager.Connect(client, ip, port);
	}

	void GameManager::Process(int threadCount)
	{
		_networkManager.Process(threadCount);
	}

}