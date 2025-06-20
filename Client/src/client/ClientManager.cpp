#pragma once


#include "client/ClientManager.h"
#include "../utility/Debug.h"
#include "../utility/ConstValue.h"




namespace Client
{
	ClientManager::ClientManager()
	{
		Utility::Log("Client", "ClientManager", "Construct");
	}

	ClientManager::~ClientManager()
	{
		Utility::Log("Client", "ClientManager", "Destruct");
	}

	void ClientManager::Initialize(std::string ip, int port, int clientCount, int threadCount)
	{
		Utility::Log("Client", "ClientManager", "Initialize");

		int overlappedCountMax = Utility::ConstValue::GetInstance().OverlappedCountMax;
		_overlappedQueue.Construct(overlappedCountMax);
		for (int i = 0;i < overlappedCountMax; ++i)
		{
			auto overlapped = new Network::CustomOverlapped();
			_overlappedQueue.push(std::move(overlapped));
		}
		Utility::Log("Client", "ClientManager", "OverlappedQueue Ready");

		WSADATA wsaData;

		// Winsock �ʱ�ȭ
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "WSAStartup ����: " << WSAGetLastError() << "\n";
			return;
		}

		SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		// ConnectEx �Լ� ������ ��������
		LPFN_CONNECTEX connectEx = NULL;
		GUID connectExGuid = WSAID_CONNECTEX;
		DWORD bytes;
		if (WSAIoctl(clientSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&connectExGuid, sizeof(connectExGuid),
			&connectEx, sizeof(connectEx),
			&bytes, NULL, NULL))
		{
			Utility::LogError("Client", "ClientManager", "WSAIoctl ����: " + std::to_string(WSAGetLastError()));
			closesocket(clientSocket);
			return;
		}

		// ���� �ּ� ����
		sockaddr_in serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

		// ���� �ּ� ���ε�
		sockaddr_in localAddr = { 0 };
		localAddr.sin_family = AF_INET;
		localAddr.sin_addr.s_addr = INADDR_ANY;
		localAddr.sin_port = 0;  // �ڵ� �Ҵ�


		for (int i = 0;i < clientCount; ++i)
		{
			SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (newSocket == INVALID_SOCKET)
			{
				Utility::LogError("Client", "ClientManager", "���� ���� ����: " + std::to_string(WSAGetLastError()));
				continue;
			}

			if (bind(newSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
			{
				Utility::LogError("Client", "ClientManager", "Ŭ���̾�Ʈ bind ����: " + std::to_string(WSAGetLastError()));
				closesocket(newSocket);
				continue;
			}

			auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);

			auto client = std::make_shared<Network::Client>();
			client->Initialize(socketSharedPtr);
			auto overlappedPtr = _overlappedQueue.pop();
			overlappedPtr->Clear();
			client->ConnectEx(connectEx, serverAddr, *overlappedPtr);

			_clientMap.insert({ reinterpret_cast<ULONG_PTR>(socketSharedPtr.get()), client });
		}

		Utility::Log("Client", "ClientManager", "Client Count: " + std::to_string(_clientMap.size()) + " Initialize Success");
	}
}