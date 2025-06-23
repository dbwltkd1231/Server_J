#pragma once
#include "network/NetworkManager.h"
#include "../utility/Debug.h"

#include "utility/ConstValue.h"

namespace Network
{
	NetworkManager::NetworkManager()
	{
		Utility::Log("Network", "NetworkManager", "Construct");
	}

	NetworkManager::~NetworkManager()
	{
		Utility::Log("Network", "NetworkManager", "Destruct");
	}

	void NetworkManager::Connect(std::shared_ptr<Network::Client> targetClient, std::string ip, int port)
	{
		_authModule = std::make_shared<NetManagerModule>();
		_authModule->Initialize(ip, port);

		SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);
		targetClient->Initialize(socketSharedPtr);

		bool result = _authModule->Connect(targetClient, socketSharedPtr, ClientUtility::ConstValue::GetInstance().ThreadCount);
		if (!result)
		{
			Utility::LogError("Game", "GameManager", "Socket - IOCP CONNET FAIL");
			return;
		}
	}

	void NetworkManager::Process(int threadCount)
	{
		_authModule->Process(threadCount);
	}
}