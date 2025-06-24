#pragma once
#include <string>

#include "network/NetManagerModule.h"


namespace Network
{
	class NetworkManager
	{
	public:
		NetworkManager();
		~NetworkManager();

	public:
		void Initialze(std::string ip, int port);
		void CallbackSetting(
			std::function<void(Network::ServerType&, ULONG_PTR&, std::shared_ptr<Network::Client>)>& acceptCallback,
			std::function<void(Network::ServerType&, ULONG_PTR&, CustomOverlapped*)>& receiveCallback,
			std::function<void(Network::ServerType&, ULONG_PTR& socket, int bytesTransferred, int errorCode)>& disconnectCallback
		);
		void ConnectAuthServer(std::shared_ptr<Network::Client> targetClient, Network::CustomOverlapped* overlappedPtr);
		void ReceiveReadyToModule(Network::ServerType& serverType, ULONG_PTR& targetSocket, Network::CustomOverlapped* overlappedPtr);
		void Process(int threadCount);
	private:
		void Work();
	private:
		std::shared_ptr<Network::NetManagerModule> _authModule;

	};
}