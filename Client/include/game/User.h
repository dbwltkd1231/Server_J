#pragma once
#include <iostream>
#include <string>

#include "network/Client.h"
#include "../auth/AuthProtocol.h"
#include "../lobby/NetworkProtocol.h"

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
		void SetAccountData(std::string authToken, int64_t accountNumber, std::string userId);
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
		int _money;
		//TODO 인벤토리
		int _ranking;//선택사항 -> 후순위로
	};
}