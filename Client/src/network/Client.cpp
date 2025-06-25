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

	bool Client::operator==(const Client& other) const
	{
		return reinterpret_cast<ULONG_PTR>(ClientSocketPtr.get()) ==
			reinterpret_cast<ULONG_PTR>(other.ClientSocketPtr.get());
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

	ULONG_PTR Client::GetSocketPtr()
	{
		if (ClientSocketPtr == nullptr)
			return 0;

		return (ULONG_PTR)ClientSocketPtr.get();
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

	void Client::Send(CustomOverlapped* overlappedPtr, const MessageHeader header, std::string& stringBuffer, int& bodySize)
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
			std::string log = "WSASend ����! ���� �ڵ�: " + std::to_string(errorCode);
			Utility::Log("Network", "Client", log);
			return;
		}

		Utility::Log("Network", "Client", "Ŭ���̾�Ʈ WSASend ȣ��");
	}
} 