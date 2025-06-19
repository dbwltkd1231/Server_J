#pragma once
#include <memory>

#include <winsock2.h>

#include "CustomOverlapped.h"

namespace Network
{
	class Client
	{
	public:
		Client();
		~Client();

	public:
		void Initialize(std::shared_ptr<SOCKET> clientSocketPtr);
		void Deinitialize();

	public:
		bool AcceptEx(SOCKET& listenSocket, LPFN_ACCEPTEX& acceptExPtr, CustomOverlapped* overlappedPtr);
		void ReceiveReady(CustomOverlapped* overlappedPtr);
		void Send(CustomOverlapped* overlappedPtr, const MessageHeader header, std::string& stringBuffer, int& bodySize);

	public:
		std::shared_ptr<SOCKET> ClientSocketPtr;

	};

}