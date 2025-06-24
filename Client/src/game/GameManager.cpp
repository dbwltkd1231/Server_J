#pragma once
#include "Game/GameManager.h"
#include "../utility/ConstValue.h"
#include "../game/NetworkProtocol.h"
#include "../game/BasicData.h"

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

	void GameManager::Initialize(std::string ip, int port, int clientCount)
	{
		_networkManager.Initialze(ip, port);

		_acceptCallback = std::bind(&GameManager::AcceptCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		_receiveCallback = std::bind(&GameManager::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		_disconnectCallback = std::bind(&GameManager::DisconnectCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		
		_networkManager.CallbackSetting(_acceptCallback, _receiveCallback, _disconnectCallback);

		_overlappedQueue = std::make_shared<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>>();
		_overlappedQueue->Construct(Utility::ConstValue::GetInstance().OverlappedCountMax);
		for (int i = 0;i < Utility::ConstValue::GetInstance().OverlappedCountMax; ++i)
		{
			auto overlappedPtr = new Network::CustomOverlapped();
			_overlappedQueue->push(std::move(overlappedPtr));
		}

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

	// Callback functions for network events
	void GameManager::AcceptCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, std::shared_ptr<Network::Client> client)
	{
		auto overlappedPtr = _overlappedQueue->pop();
		overlappedPtr->Clear();

		_networkManager.ReceiveReadyToModule(targetServer, targetSocket, overlappedPtr);

		if (targetServer == Network::ServerType::Auth)
		{
			Game::User user;

		}
		else if (targetServer == Network::ServerType::Lobby)
		{

		}
	}

	void GameManager::DisconnectCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, int bytesTransferred, int errorCode)
	{

	}

	void GameManager::ReceiveCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, Network::CustomOverlapped* overlappedPtr)
	{
		Network::MessageHeader* messageHeader = reinterpret_cast<Network::MessageHeader*>(overlappedPtr->Wsabuf[0].buf);

		auto bodySize = ntohl(messageHeader->BodySize);
		auto contentsType = ntohl(messageHeader->ContentsType);
		auto bufferString = std::string(overlappedPtr->Wsabuf[1].buf, bodySize);
		ReadMessage(targetSocket, contentsType, bufferString);

		overlappedPtr->Clear();
		_overlappedQueue->push(std::move(overlappedPtr));
	}

	void GameManager::ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string& stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();

		switch (messageType)
		{
			case protocol::MessageContent_RESPONSE_CONNECT:
			{
				auto responseConnect = flatbuffers::GetRoot<protocol::RESPONSE_CONNECT>(buffer);
				
				std::string authToken = responseConnect->auth_token()->str();
				bool loginSuccess = responseConnect->login_success();
				int lobbyPort = responseConnect->loby_port();
			
				//User °´Ã¼¸¸µé±â
				break;
			}
		}
	}
}