#pragma once
#include "Game/GameManager.h"
#include "../utility/ConstValue.h"
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

	// Callback functions for network events
	void GameManager::AcceptCallback(Network::ServerType targetServer, ULONG_PTR targetSocket)
	{

	}

	void GameManager::DisconnectCallback(Network::ServerType targetServer, ULONG_PTR targetSocket, int bytesTransferred, int errorCode)
	{

	}

	void GameManager::ReceiveCallback(Network::ServerType targetServer, ULONG_PTR targetSocket, Network::CustomOverlapped* overlappedPtr)
	{
		
	}


	void GameManager::Initialize(std::string ip, int port, int clientCount)
	{
		_authCallback = std::bind(&GameManager::AcceptCallback, this, std::placeholders::_1, std::placeholders::_2);
		_receiveCallback = std::bind(&GameManager::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		_disconnectCallback = std::bind(&GameManager::DisconnectCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		
		_overlappedQueue = std::make_shared<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>>();
		_overlappedQueue->Construct(Utility::ConstValue::GetInstance().OverlappedCountMax);
		for (int i = 0;i < Utility::ConstValue::GetInstance().OverlappedCountMax; ++i)
		{
			auto overlappedPtr = new Network::CustomOverlapped();
			_overlappedQueue->push(std::move(overlappedPtr));
		}

		_networkManager.Initialze(ip, port);
		_networkManager.CallbackSetting(_authCallback, _receiveCallback, _disconnectCallback);

		for (int i = 0;i < clientCount;++i)
		{
			auto overlappedPtr = _overlappedQueue->pop();
			overlappedPtr->Clear();
			auto client = std::make_shared<Network::Client>();
			_networkManager.ConnectAuthServer(client, overlappedPtr);
		}

		Utility::Log("Game", "GameManager", "Initialize with IP: " + ip + ", Port: " + std::to_string(port) + ", Client Count: " + std::to_string(clientCount));
	}

	void GameManager::Process(int threadCount)
	{
		_networkManager.Process(threadCount);
	}

}