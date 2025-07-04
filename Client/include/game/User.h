#pragma once
#include <iostream>
#include <string>
#include "network/NetworkManager.h"
#include "oneapi/tbb/concurrent_map.h"
#include "../auth/AuthProtocol.h"
#include "../lobby/LobbyProtocol.h"

namespace Game
{
	struct InventorySlot
	{
		std::string Guid;
		long ItemSeed;
		int ItemCount;
	};

	class User
	{
	public:
		User();
		~User();

	public:
		void Initialize(std::shared_ptr<Network::Client> client);
		void Deinitialize();

	public:
		void RequestConnect(Network::CustomOverlapped* sendOverlappedPtr);
		void RequestLogIn(Network::CustomOverlapped* sendOverlappedPtr);

	public:
		void Login();
		void Logout();
		void SetAccountData(int64_t accountNumber, std::string userID, std::string authToken);
		void SetUserData(int64_t money, int ranking, int inventoryCapacity);
		void UpdateInventory(std::string guid, long itemSeed, int itemCount);
		void ItemBreak(std::string guid, int removeCount, int moneyReward);
		long GetAccountNumber();
		void BreakRandomItem(Network::CustomOverlapped* sendOverlappedPtr);
		
	public:
		std::shared_ptr<Network::Client> ClientPtr;

	private:
		bool _isLogin;

	private:
		std::string _authToken;
		int64_t _accountNumber;
		std::string _userID;
		int64_t _money;
		int _ranking;//선택사항 -> 후순위로
		int _inventoryCapacity;
		tbb::concurrent_map<std::string, InventorySlot> _inventoryMap;
	};
}