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

	void NetworkManager::Initialze(std::string ip, int port)
	{
		Utility::Log("Network", "NetworkManager", "Initialize");
		_authModule = std::make_shared<NetManagerModule>();
		_authModule->Initialize(ip, port, Network::ServerType::Auth);
	}

	void NetworkManager::CallbackSetting(
		std::function<void(Network::ServerType&, ULONG_PTR&, std::shared_ptr <Network::Client>)>& acceptCallback,
		std::function<void(Network::ServerType&, ULONG_PTR&, CustomOverlapped*)>& receiveCallback,
		std::function<void(Network::ServerType&, ULONG_PTR& socket, int bytesTransferred, int errorCode)>& disconnectCallback
	)
	{
		_authModule->CallbackSetting(acceptCallback, receiveCallback, disconnectCallback);
	}


	void NetworkManager::ConnectAuthServer(std::shared_ptr<Network::Client> targetClient, Network::CustomOverlapped* overlappedPtr)
	{
		SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);
		targetClient->Initialize(socketSharedPtr);

		bool result = _authModule->Connect(targetClient, socketSharedPtr, ClientUtility::ConstValue::GetInstance().ThreadCount, overlappedPtr);
		if (!result)
		{
			Utility::LogError("Game", "GameManager", "Socket - IOCP CONNET FAIL");
			return;
		}
	}

	void NetworkManager::ReceiveReadyToModule(Network::ServerType& serverType, ULONG_PTR& targetSocket, Network::CustomOverlapped* overlappedPtr)
	{
		if (serverType == Network::ServerType::Auth)
		{
			_authModule->ReceiveReadyToClient(targetSocket, overlappedPtr);
		}

	}

	void NetworkManager::Process(int threadCount)
	{
		_authModule->Process(threadCount);
	}
}