#pragma once

#include "Game/GameManager.h"

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"


namespace Game
{
	GameManager::GameManager()
	{
		_isOn = false;
		Utility::Log("Game", "GameManager", "Construct");
	}

	GameManager::~GameManager()
	{
		_isOn = false;
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

		Sleep(1000); // 세팅 대기..

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
		_isOn = true;
		_networkManager.Process(threadCount);

		while (_isOn)
		{

		}
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

		if (targetServer == Network::ServerType::Auth)
		{
			Utility::Log("Client", "GameManager", " 인증서버 접속해제");
		}
		else if (targetServer == Network::ServerType::Lobby)
		{
			//로그아웃처리
			Utility::Log("Client", "GameManager", " 로비서버 접속해제");
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
					user->SetAccountData(accountNumber, uid, authToken);
					user->Deinitialize();
					_socketUserMap->unsafe_erase(targetSocket); //User객체가 사용하는 소켓 바꿔주기 전 socket은 맵에서 삭제.

					Sleep(1000); // 클라이언트 초기화 대기...(clostsocket이 포함되어있기때문)

					auto client = std::make_shared<Network::Client>();
					client->Initialize();
					user->Initialize(client);

					auto socketPtr = (ULONG_PTR)client->GetSocket().get();
					_socketUserMap->insert({ socketPtr, user });
					//TODO 포트번호를 여기서 받아야함.

					auto overlappedPtr = _overlappedQueue->pop();
					overlappedPtr->Clear();

					_networkManager.ConnectLobbyServer(client, overlappedPtr);
				}
				break;
			}
		}
	}

	void GameManager::ReadLobbyMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string& stringValue)
	{
		std::shared_ptr<Game::User> targetUser = nullptr;
		auto finder = _socketUserMap->find(targetSocket);
		if (finder == _socketUserMap->end())
		{
			return;
		}
		targetUser = finder->second;

		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();
		std::string log;

		switch (messageType)
		{
			case protocol::MessageContent_RESPONSE_LOGIN:
			{
				auto responseLogin = flatbuffers::GetRoot<protocol::RESPONSE_LOGIN>(buffer);
				auto detail = responseLogin->detail();
				auto feedback = responseLogin->feedback();

				std::string feedbackStr = (detail == 1 ? "Success" : "Fail");
				log = "RESPONSE_LOGIN" + feedbackStr;
				Utility::Log("Game", "GameManager", log);

				targetUser->Login();
				break;
			}

			case protocol::MessageContent_NOTICE_ACCOUNT:
			{
				auto responseLogin = flatbuffers::GetRoot<protocol::NOTICE_ACCOUNT>(buffer);
				auto accountUID = responseLogin->user_id()->str();
				auto gameMoney = responseLogin->money();
				auto gameMoneyRank = responseLogin->ranking();
				auto inventoryCapacity = responseLogin->inventory_capacity();

				log = "NOTICE_ACCOUNT | UID: " + accountUID + ", Money: " + std::to_string(gameMoney) +
					", Rank: " + std::to_string(gameMoneyRank) +
					", Inventory: " + std::to_string(inventoryCapacity);

				Utility::Log("Game", "GameManager", log);
				targetUser->SetUserData(gameMoney, gameMoneyRank, inventoryCapacity);
				break;
			}

			case protocol::MessageContent_NOTICE_INVENTORY:
			{
				auto noticeInventory = flatbuffers::GetRoot<protocol::NOTICE_INVENTORY>(buffer);

				int inventoryTotalCount = noticeInventory->inventory_total_count();

				for (int i = 0; i < inventoryTotalCount; ++i)
				{
					std::string guid = noticeInventory->inventory_slots()->Get(i)->guid()->str();
					long itemSeed = noticeInventory->inventory_slots()->Get(i)->item_seed();
					int itemCount = noticeInventory->inventory_slots()->Get(i)->count();


					targetUser->UpdateInventory(guid, itemSeed, itemCount);

					log = "NOTICE_INVENTORY | UID: " + std::to_string(targetUser->GetAccountNumber()) + ", Guid: " + guid +
						", ItemSeed: " + std::to_string(itemSeed) +
						", ItemCount: " + std::to_string(itemCount);

					Utility::Log("Game", "GameManager", log);
				}

				if (inventoryTotalCount < 1)
				{
					Utility::Log("Game", "GameManager", "NOTICE_INVENTORY : 0");
				}

				break;
			}

			case protocol::MessageContent_NOTICE_INVENTORY_UPDATE:
			{
				auto noticeInventoryUpdate = flatbuffers::GetRoot<protocol::NOTICE_INVENTORY_UPDATE>(buffer);

				int inventoryUpdateTotalCount = noticeInventoryUpdate->inventory_total_count();

				for (int i = 0; i < inventoryUpdateTotalCount; ++i)
				{
					std::string guid = noticeInventoryUpdate->inventory_slots()->Get(i)->guid()->str();
					long itemSeed = noticeInventoryUpdate->inventory_slots()->Get(i)->item_seed();
					int itemCount = noticeInventoryUpdate->inventory_slots()->Get(i)->count();


					targetUser->UpdateInventory(guid, itemSeed, itemCount);
				
					log = "NOTICE_INVENTORY_UPDATE | UID: " + std::to_string(targetUser->GetAccountNumber()) + ", Guid: " + guid +
						", ItemSeed: " + std::to_string(itemSeed) +
						", ItemCount: " + std::to_string(itemCount);

					Utility::Log("Game", "GameManager", log);
					auto overlappedPtr = _overlappedQueue->pop();
					overlappedPtr->Clear();
					targetUser->BreakRandomItem(overlappedPtr);
				}

				if (inventoryUpdateTotalCount < 1)
				{
					Utility::Log("Game", "GameManager", "NOTICE_INVENTORY_UPDATE : 0");
				}

				break;
			}

			case protocol::MessageContent_RESPONSE_ITEM_BREAK:
			{
				auto responseItemBreak = flatbuffers::GetRoot<protocol::RESPONSE_ITEM_BREAK>(buffer);

				std::string successStr = (responseItemBreak->feedback() == true ? "TRUE" : "FALSE");
				std::string guid = responseItemBreak->guid()->str();
				int moneyReward = responseItemBreak->money_reward();
				int removeCount = responseItemBreak->remove_count();

				//TODO seed기반 업데이트요청필요...
				Utility::Log("Game", "GameManager", "RESPONSE_ITEM_BREAK " + successStr);

				if (responseItemBreak->feedback() == true)
				{
					targetUser->ItemBreak(guid, moneyReward, removeCount);
				}
			}
		}
	}
}