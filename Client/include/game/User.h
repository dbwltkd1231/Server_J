#pragma once
#include <iostream>
#include <string>

#include "network/Client.h"
#include "../auth/AuthProtocol.h"
#include "../lobby/LobbyProtocol.h"

namespace Game
{
	class User
	{
	public:
		User();
		~User();

	public:
		void Initialize(std::shared_ptr<Network::Client> client);
		void Deinitialize();

	public:
		void SetAccountData(int64_t accountNumber, std::string userID, std::string authToken);
		void SetUserData(int64_t money, int ranking, int inventoryCapacity);
		void RequestConnect(Network::CustomOverlapped* sendOverlappedPtr);
		void RequestLogIn(Network::CustomOverlapped* sendOverlappedPtr);

	public:
		std::shared_ptr<Network::Client> ClientPtr;
		//Network::NetworkManager _authManager;
		//Network::NetworkManager _lobbyManager;

	private:
		std::string _authToken;
		int64_t _accountNumber;
		std::string _userID;
		int64_t _money;
		//TODO 인벤토리
		int _ranking;//선택사항 -> 후순위로
		int _inventoryCapacity;
	};
}