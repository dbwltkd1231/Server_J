#pragma once
#include "Client.h"
#include "Debug.h"

namespace Network
{
	Client::Client()
	{
		ClientSocketPtr = nullptr;
	}
	Client::~Client()
	{
		ClientSocketPtr = nullptr;
	}

	void Client::Initialize(std::shared_ptr<SOCKET> clientSocketPtr)
	{
		ClientSocketPtr = clientSocketPtr;
		Utility::Log("Network", "Client", "Initialize");
	}

	void Client::Deinitialize()
	{
		ClientSocketPtr = nullptr;
		Utility::Log("Network", "Client", "Deinitialize");
	}

	bool Client::AcceptEx(SOCKET& listenSocket, LPFN_ACCEPTEX& acceptExPtr, CustomOverlapped* overlappedPtr)
	{
		int errorCode;
		int errorCodeSize = sizeof(errorCode);
		getsockopt(*ClientSocketPtr, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorCodeSize);
		if (errorCode != 0)
		{
			std::cerr << "Socket error detected: " << errorCode << std::endl;
			return false;
		}

		overlappedPtr->SetOperationType(Network::OperationType::OP_ACCEPT);
		overlappedPtr->SetSocket(ClientSocketPtr);

		DWORD bytesReceived = 0;
		bool result = acceptExPtr(listenSocket, *ClientSocketPtr, overlappedPtr->Wsabuf[1].buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &bytesReceived, (CustomOverlapped*)&(*overlappedPtr));

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = "AcceptEx 실패! 오류 코드: " + std::to_string(errorCode);
		}
		else
		{
			log = "Socket Accept Ready";
		}

		Utility::Log("Network", "Client", log);
	}


	void Client::ReceiveReady(CustomOverlapped* overlappedPtr)
	{
		int errorCode;
		int errorCodeSize = sizeof(errorCode);
		getsockopt(*ClientSocketPtr, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorCodeSize);
		if (errorCode != 0)
		{
			std::cerr << "Socket error detected: " << errorCode << std::endl;
			return;
		}

		overlappedPtr->SetOperationType(Network::OperationType::OP_RECV);

		DWORD flags = 0;
		int result = WSARecv(*ClientSocketPtr, overlappedPtr->Wsabuf, 2, nullptr, &flags, &*overlappedPtr, nullptr);

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = "WSARecv 실패! 오류 코드: " + std::to_string(errorCode);
		}
		else
		{
			log = " Socket Receive Ready";
		}

		Utility::Log("Network", "Client", log);
	}


	void Client::Send(CustomOverlapped* overlappedPtr, const MessageHeader header, std::string& stringBuffer, int bodySize)
	{
		if (ClientSocketPtr == nullptr || *ClientSocketPtr == INVALID_SOCKET)
		{
			Utility::Log("Network", "Client", "Invalid Socket Pointer");
			return;
		}

		overlappedPtr->SetOperationType(Network::OperationType::OP_SEND);
		overlappedPtr->SetHeader(header);
		overlappedPtr->SetBody(stringBuffer.c_str(), bodySize);

		DWORD flags = 0;
		int result = WSASend(*ClientSocketPtr, overlappedPtr->Wsabuf, 2, nullptr, flags, &*overlappedPtr, nullptr);
		int errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::string log = "WSASend 실패! 오류 코드: " + std::to_string(errorCode);
			Utility::Log("Network", "Client", log);
			return;
		}

		Utility::Log("Network", "Client", "Socket Send");
	}
}