#pragma once

#include "network/Client.h"
#include "../utility/Debug.h"
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
} 