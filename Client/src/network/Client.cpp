#pragma once

#include "network/Client.h"

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


	void Client::ConnectEx(LPFN_CONNECTEX& connectEx, sockaddr_in serverAddr, Network::CustomOverlapped& overlapped)
	{
		// ������ ��ȿ���� ���� Ȯ��
		if (ClientSocketPtr == nullptr || *ClientSocketPtr == INVALID_SOCKET) 
		{
			Utility::LogError("Network", "Client", "ConnectEx ����: �߸��� ���� ������");
			return;
		}

		overlapped.SetOperationType(Network::OperationType::OP_ACCEPT);
		overlapped.SetSocket(ClientSocketPtr);
		overlapped.hEvent = NULL;

		BOOL result = connectEx(*ClientSocketPtr, (sockaddr*)&serverAddr, sizeof(serverAddr), NULL, 0, NULL, &overlapped);

		if (!result)
		{
			int errorCode = WSAGetLastError();
			if (errorCode != WSA_IO_PENDING) 
			{
				Utility::LogError("Network", "Client", "ConnectEx ����! ���� �ڵ�: " + std::to_string(errorCode));

				// ������ �����ϰ� �ݰ� ����
				if (*ClientSocketPtr != INVALID_SOCKET) 
				{
					closesocket(*ClientSocketPtr);
					*ClientSocketPtr = INVALID_SOCKET;
				}
				WSACleanup();
				return;
			}
		}

		Utility::Log("Network", "Client", "ConnectEx ���۵�, WSA_IO_PENDING ����");
	}

	void Client::ReceiveReady(CustomOverlapped& overlapped)
	{
		int errorCode;
		int errorCodeSize = sizeof(errorCode);
		getsockopt(*ClientSocketPtr, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorCodeSize);
		if (errorCode != 0)
		{
			std::cerr << "Socket error detected: " << errorCode << std::endl;
			return;
		}

		overlapped.Clear();
		overlapped.SetOperationType(OperationType::OP_RECV);

		MessageHeader newHeader(0, 0);

		DWORD flags = 0;
		int result = WSARecv(*ClientSocketPtr, overlapped.Wsabuf, 2, nullptr, &flags, &overlapped, nullptr);

		std::string log;
		errorCode = WSAGetLastError();
		if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
		{
			log = "WSARecv ����! ���� �ڵ�: " + std::to_string(errorCode);
			Utility::LogError("Network", "Client", log);
		}
		else
		{
			log = " Socket Receive Ready";
			Utility::Log("Network", "Client", log);
		}
	}
} 