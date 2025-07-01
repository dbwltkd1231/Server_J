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
		bool operator==(const Client& other) const;
	private:
		std::shared_ptr<SOCKET> ClientSocketPtr;

	public:
		void Initialize();
		void Deinitialize();
		void ConnectEx(LPFN_CONNECTEX& connectEx, sockaddr_in serverAddr, Network::CustomOverlapped& overlapped);
		void ReceiveReady(CustomOverlapped& overlapped);
		void Send(CustomOverlapped* overlappedPtr, const MessageHeader header, std::string& stringBuffer, int& bodySize);

	public:
		std::shared_ptr<SOCKET> GetSocket();
	};
}
