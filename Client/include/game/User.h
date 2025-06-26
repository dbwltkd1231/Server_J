#pragma once
#include <iostream>
#include <string>

#include "network/Client.h"
#include "../auth/NetworkProtocol.h"

namespace Game
{
	class User
	{
	public:
		User();
		~User();

	public:
		void Initialize(std::shared_ptr<Network::Client> client, Network::CustomOverlapped* sendOverlappedPtr);

	private:
		//std::shared_ptr<Network::NetManagerModule> _authModule;
	private:
		std::shared_ptr<Network::Client> _client;
		//Network::NetworkManager _authManager;
		//Network::NetworkManager _lobbyManager;
	private:
		std::string _authToken;

	public:
		//void AuthLogic();
		//void Initialize(int64_t accountNumber, std::string userID, int money);
		//void Deinitialize();
	private:
		int64_t _accountNumber;
		std::string _userID;
		int _money;
		//TODO 인벤토리
		int _ranking;//선택사항 -> 후순위로
	};
}