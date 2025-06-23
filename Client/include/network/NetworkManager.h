#pragma once
#include <string>

#include "game/User.h"
#include "network/NetManagerModule.h"


namespace Network
{
	class NetworkManager
	{
	public:
		NetworkManager();
		~NetworkManager();

	public:
		void Connect(std::shared_ptr<Network::Client> targetClient, std::string ip, int port);
		void Process(int threadCount);
	private:
		void Work();
	private:
		std::shared_ptr<Network::NetManagerModule> _authModule;

	};
}