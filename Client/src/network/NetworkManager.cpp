#pragma once
#include "network/NetworkManager.h"
#include "../utility/Debug.h"

#include "game/ConstValue.h"

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

	void NetworkManager::Initialze()
	{
		Utility::Log("Network", "NetworkManager", "Initialize");
		_authModule = std::make_shared<NetManagerModule>();
		_authModule->Initialize(Game::ConstValue::GetInstance().IP, Game::ConstValue::GetInstance().AuthServerPort, Network::ServerType::Auth);

		_lobbyModule = std::make_shared<NetManagerModule>();
		_lobbyModule->Initialize(Game::ConstValue::GetInstance().IP, Game::ConstValue::GetInstance().LobbyServerPort, Network::ServerType::Lobby);
	}

	void NetworkManager::CallbackSetting(
		std::function<void(Network::ServerType&, std::shared_ptr <Network::Client>, Network::CustomOverlapped*)>& acceptCallback,
		std::function<void(Network::ServerType&, ULONG_PTR&, CustomOverlapped*)>& receiveCallback,
		std::function<void(Network::ServerType&, ULONG_PTR& socket, int bytesTransferred, int errorCode, CustomOverlapped*)>& disconnectCallback,
		std::function<void(Network::CustomOverlapped*)>& sendCallback
	)
	{
		_authModule->CallbackSetting(acceptCallback, receiveCallback, disconnectCallback, sendCallback);
		_lobbyModule->CallbackSetting(acceptCallback, receiveCallback, disconnectCallback, sendCallback);
	}


	void NetworkManager::ConnectAuthServer(std::shared_ptr<Network::Client> targetClient, Network::CustomOverlapped* overlappedPtr)
	{
		bool result = _authModule->Connect(targetClient, targetClient->GetSocket(), Game::ConstValue::GetInstance().ThreadCount, overlappedPtr);
		if (!result)
		{
			Utility::LogError("Game", "GameManager", "Socket - IOCP CONNET FAIL");
			return;
		}
	}

	void NetworkManager::ConnectLobbyServer(std::shared_ptr<Network::Client> targetClient, Network::CustomOverlapped* overlappedPtr)
	{
		bool result = _lobbyModule->Connect(targetClient, targetClient->GetSocket(), Game::ConstValue::GetInstance().ThreadCount, overlappedPtr);
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
		else if (serverType == Network::ServerType::Lobby)
		{
			_lobbyModule->ReceiveReadyToClient(targetSocket, overlappedPtr);
		}
	}

	void NetworkManager::Process(int threadCount)
	{
		_authModule->Process(threadCount);
		_lobbyModule->Process(threadCount);
	}
}