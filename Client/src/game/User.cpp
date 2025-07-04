#pragma once
#include "game/User.h"
#include "../utility/Debug.h"
#include "game/ConstValue.h"

namespace Game
{
	User::User()
	{
		_isLogin = false;
	}

	User::~User()
	{
		_isLogin = false;
	}

	void User::Initialize(std::shared_ptr<Network::Client> client)
	{
		ClientPtr = client;
	}

	void User::Deinitialize()
	{
		ClientPtr->Deinitialize();

		ClientPtr = nullptr;
	}

	void User::SetAccountData(int64_t accountNumber, std::string userID, std::string authToken)
	{
		_userID = userID;
		_authToken = authToken;
		_accountNumber = accountNumber;
	}

	void User::Login()
	{
		_isLogin = true;
	}

	void User::Logout()
	{
		_isLogin = false;
	}

	void User::SetUserData(int64_t money, int ranking, int inventoryCapacity)
	{
		_money = money;
		_ranking = ranking;
		_inventoryCapacity = inventoryCapacity;
	}

	void User::UpdateInventory(std::string guid, long itemSeed, int itemCount)
	{
		auto finder = _inventoryMap.find(guid);
		if (finder == _inventoryMap.end())
		{
			_inventoryMap.insert(std::make_pair(guid, InventorySlot{ guid , itemSeed, itemCount }));
		}
		else
		{
			auto inventorySlot = finder->second;
			inventorySlot.ItemSeed = itemSeed;
			inventorySlot.ItemCount = itemCount;
		}
	}

	void User::ItemBreak(std::string guid, int removeCount, int moneyReward)
	{
		auto finder = _inventoryMap.find(guid);
		if (finder == _inventoryMap.end())
		{
			return;
		}

		auto inventorySlot = finder->second;
		inventorySlot.ItemCount -= removeCount;
		Utility::Log("Game", "User", "InventoryUpdate : " + std::to_string(inventorySlot.ItemSeed) +" " + std::to_string(inventorySlot.ItemCount));

		if (inventorySlot.ItemCount < 1)
		{
			_inventoryMap.unsafe_erase(inventorySlot.Guid);
		}

		_money += moneyReward;
		Utility::Log("Game", "User", "MoneyUpdate : " + std::to_string(_money));
	}

	long User::GetAccountNumber()
	{
		return _accountNumber;
	}

	void User::BreakItem(std::string guid, Network::CustomOverlapped* sendOverlappedPtr)
	{
		if (!_isLogin)
			return;

		if (_inventoryMap.size() < 1)
			return;

		//현재시간에서 초+분을 더한값이 짝수면 아이템 브레이크
		std::time_t now = std::time(nullptr);
		std::tm localTime;

		localtime_s(&localTime, &now);
		int sum = localTime.tm_min + localTime.tm_sec;
		if (sum % 2 == 0)
		{
			auto finder = _inventoryMap.find(guid);
			if (finder != _inventoryMap.end())
			{
				auto inventorySlot = finder->second;
				if (inventorySlot.ItemCount > 0)
				{
					int removeCount = 1;
					Common::Lobby::PacketOutput output;
					Common::Lobby::RequestItemBreak(_accountNumber, inventorySlot.Guid, removeCount, output);
					Network::MessageHeader newHeader(htonl(output.BodySize), htonl(output.ContentsType));

					ClientPtr->Send(sendOverlappedPtr, newHeader, output.Buffer, output.BodySize);
					Utility::Log("Client", "User", "REQUEST_ITEM_BREAK " +std::to_string(_accountNumber) + " " + std::to_string(inventorySlot.ItemSeed) + " " + std::to_string(removeCount));
				}
			}
		}
	}

	void User::RequestConnect(Network::CustomOverlapped* sendOverlappedPtr)
	{
		uint32_t contentsType = 0;
		std::string stringBuffer = "";
		int bodySize = 0;

		int clientNumber = Game::ConstValue::GetInstance().CurrentClinetIndex.fetch_add(1, std::memory_order_relaxed);
		std::string uid = Game::ConstValue::GetInstance().TestUID + std::to_string(clientNumber);

		Common::Auth::CreateRequestConnect(uid, contentsType, stringBuffer, bodySize);
		Network::MessageHeader newHeader(htonl(bodySize), htonl(contentsType));
		ClientPtr->Send(sendOverlappedPtr, newHeader, stringBuffer, bodySize);
	}

	void User::RequestLogIn(Network::CustomOverlapped* sendOverlappedPtr)
	{
		Common::Lobby::PacketOutput output;
		Common::Lobby::CreateRequestLogIn(_accountNumber, _authToken, output);
		Network::MessageHeader newHeader(htonl(output.BodySize), htonl(output.ContentsType));

		ClientPtr->Send(sendOverlappedPtr, newHeader, output.Buffer, output.BodySize);

		Utility::Log("Client", "User", "Request LogIn");
	}
}