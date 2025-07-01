#pragma once

#include "Game/GameManager.h"

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"


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

	void GameManager::Initialize(int clientCount)
	{
		_networkManager.Initialze();

		_acceptCallback = std::bind(&GameManager::AcceptCallback, this, std::placeholders::_1, std::placeholders::_2);
		_receiveCallback = std::bind(&GameManager::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		_disconnectCallback = std::bind(&GameManager::DisconnectCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		
		_networkManager.CallbackSetting(_acceptCallback, _receiveCallback, _disconnectCallback);

		_overlappedQueue = std::make_shared<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>>();
		_overlappedQueue->Construct(Game::ConstValue::GetInstance().OverlappedCountMax);
		for (int i = 0;i < Game::ConstValue::GetInstance().OverlappedCountMax; ++i)
		{
			auto overlappedPtr = new Network::CustomOverlapped();
			_overlappedQueue->push(std::move(overlappedPtr));
		}

		for (int i = 0;i < Game::ConstValue::GetInstance().TestClientCount;++i)
		{
			auto overlappedPtr = _overlappedQueue->pop();
			overlappedPtr->Clear();
			auto client = std::make_shared<Network::Client>();
			client->Initialize();
			_networkManager.ConnectAuthServer(client, overlappedPtr);
		}

		_socketUserMap = std::make_shared<tbb::concurrent_map<ULONG_PTR, std::shared_ptr<Game::User>>>();
		Utility::Log("Client", "GameManager", "Initialize. Client Count: " + std::to_string(clientCount));
	}

	void GameManager::Process(int threadCount)
	{
		_networkManager.Process(threadCount);
	}

	// Callback functions for network events
	void GameManager::AcceptCallback(Network::ServerType& targetServer, std::shared_ptr<Network::Client> client)
	{
		auto receiveOverlappedPtr = _overlappedQueue->pop();
		receiveOverlappedPtr->Clear();

		auto socketPtr = (ULONG_PTR)client->GetSocket().get();
		_networkManager.ReceiveReadyToModule(targetServer, socketPtr, receiveOverlappedPtr);

		if (targetServer == Network::ServerType::Auth)
		{
			std::shared_ptr<Game::User> newUser = std::make_shared<Game::User>();
			newUser->Initialize(client);

			auto overlappedPtr = _overlappedQueue->pop();
			overlappedPtr->Clear();

			newUser->RequestConnect(overlappedPtr);
			_socketUserMap->insert({ socketPtr, newUser});
		}
		else if (targetServer == Network::ServerType::Lobby)
		{
			auto finder = _socketUserMap->find(socketPtr);
			if (finder != _socketUserMap->end())
			{
				auto user = finder->second;
				auto overlappedPtr = _overlappedQueue->pop();
				overlappedPtr->Clear();

				user->RequestLogIn(overlappedPtr);
			}
		}
	}

	void GameManager::DisconnectCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, int bytesTransferred, int errorCode)
	{
		auto finder = _socketUserMap->find(targetSocket);
		if (finder == _socketUserMap->end())
			return;

		finder->second->Deinitialize();
		if (targetServer == Network::ServerType::Auth)
		{
			Utility::Log("Client", "GameManager", " �������� ��������");
		}
		else if (targetServer == Network::ServerType::Lobby)
		{
			//�α׾ƿ�ó��
			Utility::Log("Client", "GameManager", " �κ񼭹� ��������");
		}

		_socketUserMap->unsafe_erase(targetSocket);
	}

	void GameManager::ReceiveCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, Network::CustomOverlapped* overlappedPtr)
	{
		auto receiveOverlappedPtr = _overlappedQueue->pop();
		receiveOverlappedPtr->Clear();
		_networkManager.ReceiveReadyToModule(targetServer, targetSocket, receiveOverlappedPtr);

		Network::MessageHeader* messageHeader = reinterpret_cast<Network::MessageHeader*>(overlappedPtr->Wsabuf[0].buf);

		auto bodySize = ntohl(messageHeader->BodySize);
		auto contentsType = ntohl(messageHeader->ContentsType);
		auto bufferString = std::string(overlappedPtr->Wsabuf[1].buf, bodySize);

		if (targetServer == Network::ServerType::Auth)
		{
			ReadAuthMessage(targetSocket, contentsType, bufferString);
		}
		else if (targetServer == Network::ServerType::Lobby)
		{
			ReadLobbyMessage(targetSocket, contentsType, bufferString);
		}
		
		overlappedPtr->Clear();
		_overlappedQueue->push(std::move(overlappedPtr));
	}

	void GameManager::ReadAuthMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string& stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();
		std::string log;
		switch (messageType)
		{
			case protocol::MessageContent_RESPONSE_CONNECT:
			{
				auto responseConnect = flatbuffers::GetRoot<protocol::RESPONSE_CONNECT>(buffer);
				long accountNumber = responseConnect->account_number();
				std::string uid = responseConnect->login_id()->str();
				std::string authToken = responseConnect->auth_token()->str();
				bool isNew = responseConnect->id_new();
				int lobbyPort = responseConnect->loby_port();
				
				log = " RESPONSE CONNECT [UID : " + uid  + " AccountNumber : " + std::to_string(accountNumber) + " New USER ? " + (isNew ? "true": "false") + " Token : " + authToken + " Port : " + std::to_string(lobbyPort) + " ]";
				Utility::Log("Game", "GameManager", log);

				auto finder = _socketUserMap->find(targetSocket);
				if (finder != _socketUserMap->end())
				{
					auto user = finder->second;
					user->Deinitialize();

					auto client = std::make_shared<Network::Client>();
					client->Initialize();
					user->Initialize(client);

					auto overlappedPtr = _overlappedQueue->pop();
					overlappedPtr->Clear();
					//TODO ��Ʈ��ȣ�� ���⼭ �޾ƾ���.
					_networkManager.ConnectLobbyServer(client, overlappedPtr);

					auto socketPtr = (ULONG_PTR)client->GetSocket().get();
					_socketUserMap->insert({ socketPtr, user });
					_socketUserMap->unsafe_erase(targetSocket); //User��ü�� ����ϴ� ���� �ٲ��ֱ� �� socket�� �ʿ��� ����.
				}
				break;
			}
		}
	}

	void GameManager::ReadLobbyMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string& stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();
		std::string log;

		switch (messageType)
		{
			case  protocol::MessageContent_RESPONSE_LOGIN:
			{
				auto responseLogin = flatbuffers::GetRoot<protocol::RESPONSE_LOGIN>(buffer);
				auto detail = responseLogin->detail();
				auto feedback = responseLogin->feedback();

				log = "RESPONSE LOGIN";
				Utility::Log("Game", "GameManager", log);
			}
		}
	}
}