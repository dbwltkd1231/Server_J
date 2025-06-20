#pragma once
#include <memory>

#include "network/CustomOverlapped.h"


namespace Network
{
	class Client
	{
	public:
		Client();
		~Client();

	private:
		std::shared_ptr<SOCKET> ClientSocketPtr;

	public:
		void Initialize(std::shared_ptr<SOCKET>);
		void Deinitialize();
		void ConnectEx(LPFN_CONNECTEX& connectEx, sockaddr_in serverAddr, Network::CustomOverlapped& overlapped);
	};
}
