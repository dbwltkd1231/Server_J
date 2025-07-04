#pragma once
#include "network/NetManagerModule.h"
#include "../utility/Debug.h"


namespace Network
{
	NetManagerModule::NetManagerModule()
	{
		if (_iocpHandle == NULL)
		{
			Utility::LogError("Client", "NetManagerModule", "IOCP Handle Creation Failed: " + std::to_string(GetLastError()));
		}
		else
		{
			Utility::Log("Client", "NetManagerModule", "IOCP Handle Created Successfully");
		}

		_isOn = false;
	}

	NetManagerModule::~NetManagerModule()
	{
		if (_iocpHandle != NULL)
		{
			CloseHandle(_iocpHandle);
			Utility::Log("Client", "NetManagerModule", "IOCP Handle Closed");
		}

		_isOn = false;
	}

	void NetManagerModule::Initialize(std::string ip, int port, ServerType serverType)
	{
		_serverType = serverType;
		_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

		// Winsock 초기화
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "WSAStartup 실패: " << WSAGetLastError() << "\n";
			return;
		}

		SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		DWORD bytes;

		// ConnectEx 함수 포인터 가져오기
		if (WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&connectExGuid, sizeof(connectExGuid),
			&connectEx, sizeof(connectEx),
			&bytes, NULL, NULL))
		{
			Utility::LogError("Client", "ClientManager", "WSAIoctl 실패: " + std::to_string(WSAGetLastError()));
			closesocket(socket);
			return;
		}

		// 서버 주소 설정
		serverAddr = { 0 };
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

		// 로컬 주소 바인딩
		localAddr = { 0 };
		localAddr.sin_family = AF_INET;
		localAddr.sin_addr.s_addr = INADDR_ANY;
		localAddr.sin_port = 0;  // 자동 할당

		_isOn = true;
	}

	void NetManagerModule::CallbackSetting(
		std::function<void(Network::ServerType&, std::shared_ptr<Network::Client>, CustomOverlapped*)> acceptCallback,
		std::function<void(Network::ServerType&, ULONG_PTR&, CustomOverlapped*)> receiveCallback,
		std::function<void(Network::ServerType&, ULONG_PTR& socket, int bytesTransferred, int errorCode, CustomOverlapped*)> disconnectCallback,
		std::function<void(CustomOverlapped*)> sendCallback
	)
	{
		_acceptCallback = acceptCallback;
		_receiveCallback = receiveCallback;
		_disconnectCallback = disconnectCallback;
		_sendCallback = sendCallback;

	}

	bool NetManagerModule::Connect(std::shared_ptr<Network::Client> targetClient, std::shared_ptr<SOCKET> targetSocket, DWORD concurrentThread, Network::CustomOverlapped* overlappedPtr)
	{
		if (*targetSocket == INVALID_SOCKET)
		{
			Utility::LogError("Client", "ClientManager", "소켓 생성 실패: " + std::to_string(WSAGetLastError()));
			return false;
		}

		if (bind(*targetSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
		{
			Utility::LogError("Client", "ClientManager", "클라이언트 bind 실패: " + std::to_string(WSAGetLastError()));
			closesocket(*targetSocket);
			return false;
		}

		auto ulongPtr = (ULONG_PTR)targetSocket.get();
		_clientMap.insert(std::make_pair(ulongPtr, targetClient));

		CreateIoCompletionPort((HANDLE)*targetSocket, _iocpHandle, ulongPtr, concurrentThread);
		targetClient->ConnectEx(connectEx, serverAddr, *overlappedPtr);

		return true;
	}

	void NetManagerModule::ReceiveReadyToClient(ULONG_PTR& targetSocket, Network::CustomOverlapped* overlappedPtr)
	{
		auto finder = _clientMap.find(targetSocket);
		if (finder == _clientMap.end())
		{
			Utility::LogError("Network", "NetManagerModule", "ReceiveReady - Client Find Fail");
			return;
		}

		finder->second->ReceiveReady(*overlappedPtr);
	}

	void NetManagerModule::Process(int threadCount)
	{
		if (!_isOn)
			return;

		for (int i = 0;i < threadCount; ++i)
		{
			std::thread sessionThread(&NetManagerModule::Work, this);
			sessionThread.detach();
		}

		Utility::Log("Client", "NetManagerModule", "Client Count: " + std::to_string(_clientMap.size()) + " Process Success");
	}

	void NetManagerModule::Work()
	{
		DWORD bytesTransferred;
		ULONG_PTR completionKey;
		Network::CustomOverlapped* overlapped = nullptr;

		while (_isOn)
		{
			bytesTransferred = 0;
			completionKey = 0;
			overlapped = nullptr;

			bool result = GetQueuedCompletionStatus(_iocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);
			if (!result)
			{
				int errorCode = WSAGetLastError();
				Utility::LogError("Network", "Session", "GetQueuedCompletionStatus Failed! Error Code: " + std::to_string(errorCode));

				switch (errorCode)
				{
				case WSAECONNRESET:
				case WSAECONNABORTED:
				case WSAENETRESET:
				case WSAETIMEDOUT:
				case WSAENOTCONN:
				case WSAESHUTDOWN:
				case ERROR_NETNAME_DELETED:
				case ERROR_CONNECTION_ABORTED:
					_clientMap.unsafe_erase(completionKey);
					_disconnectCallback(_serverType, completionKey, bytesTransferred, errorCode, overlapped);
					break;
				default:
					break;
				}

				continue;
			}

			if (result)
			{
				auto targetOverlapped = static_cast<Network::CustomOverlapped*>(overlapped);

				auto finder = _clientMap.find(completionKey);
				if (finder == _clientMap.end())
				{
					Utility::LogError("Client", "ClientManager", "Client Find Fail");

					continue;
				}

				auto client = finder->second;

				switch (targetOverlapped->GetOperation())
				{
				case Network::OperationType::OP_ACCEPT:
				{
					Utility::Log("Client", "ClientManager", "Client Connect !!");

					_acceptCallback(_serverType, client, overlapped);
					break;
				}

				case Network::OperationType::OP_RECV:
				{
					if (bytesTransferred <= 0)
					{
						_disconnectCallback(_serverType, completionKey, bytesTransferred, 0, overlapped);
						continue;
					}

					Utility::Log("Client", "ClientManager", "Client Received !!");

					_receiveCallback(_serverType, completionKey, targetOverlapped);
					break;
				}

				case Network::OperationType::OP_SEND:
				{
					Utility::Log("Client", "ClientManager", "Client Send !!");
					_sendCallback(overlapped);
					break;
				}
				}
			}

		}
	}
}
