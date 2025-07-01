#pragma once

#include "NetworkManager.h"
#include "Debug.h"



namespace Network
{
	NetworkManager::NetworkManager()
	{
		Utility::Log("Network", "IOCP", "Construct");
	}

	NetworkManager::~NetworkManager()
	{
		Utility::Log("Network", "IOCP", "Destruct");
	}

	void NetworkManager::Construct(int serverPort, int sessionCount, int overlappedCount, int clientReadyCountMax)
	{
		_clientReadyCountMax = clientReadyCountMax;
		_sessionCount = sessionCount;

		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			Utility::LogError("Network", "IOCP", "WSAStartup failed");
			return;
		}

		_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (_listenSocket == INVALID_SOCKET)
		{
			Utility::LogError("Network", "IOCP", "listenSocket Create failed");
			WSACleanup();
			return;
		}

		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(serverPort);

		if (bind(_listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			Utility::LogError("Network", "IOCP", "bind failed");
			closesocket(_listenSocket);
			WSACleanup();
			return;
		}

		Utility::Log("Network", "IOCP", "bind success");

		if (listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			Utility::LogError("Network", "IOCP", "listen failed");
			closesocket(_listenSocket);
			WSACleanup();
			return;
		}

		Utility::Log("Network", "IOCP", "listen success");

		_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, sessionCount);
		if (_handle == NULL)
		{
			Utility::LogError("Network", "IOCP", "CreateIoCompletionPort failed");
			closesocket(_listenSocket);
			WSACleanup();
			return;
		}
		Utility::Log("Network", "IOCP", "IOCP Handle Ready");

		if (!CreateIoCompletionPort((HANDLE)_listenSocket, _handle, 0, sessionCount))
		{
			Utility::LogError("Network", "IOCP", "CreateIoCompletionPort failed");
			closesocket(_listenSocket);
			WSACleanup();
			return;
		}
		Utility::Log("Network", "IOCP", "CreateIoCompletionPort Success");

		GUID guidAcceptEx = WSAID_ACCEPTEX;
		DWORD bytesReceived;
		if (WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &_acceptExPointer, sizeof(_acceptExPointer), &bytesReceived, NULL, NULL) == SOCKET_ERROR)
		{
			Utility::Log("Network", "IOCP", "WSAIoctl failed");
			return;
		}

		_overlappedQueue = std::make_shared<Utility::LockFreeCircleQueue<CustomOverlapped*>>();
		_overlappedQueue->Construct(overlappedCount);

		Sleep(1000); // ���� ���...

		for (int i = 0;i < overlappedCount; ++i)
		{
			auto overlapped = new CustomOverlapped();
			_overlappedQueue->push(std::move(overlapped));
		}
		Utility::Log("Network", "NetworkManager", "OverlappedQueue Ready Success");

		Utility::Log("Network", "NetworkManager", "Client Create Success");

		_acceptCallback = std::function<void(ULONG_PTR)>
			(
				[this]
				(ULONG_PTR targetSocket)
				{
					this->AcceptCallback(targetSocket);
				}
			);

		_receiveCallback = std::function<void(ULONG_PTR, CustomOverlapped*)>
			(
				[this]
				(ULONG_PTR targetSocket, CustomOverlapped* overlappedPtr)
				{
					this->ReceiveCallback(targetSocket, overlappedPtr);
				}
			);

		_disconnectCallback = std::function<void(ULONG_PTR, int, int)>
			(
				[this]
				(ULONG_PTR targetSocket, int bytesTransferred, int errorCode)
				{
					this->DisconnectCallback(targetSocket, bytesTransferred, errorCode);
				}
			);

		for (int i = 0;i < sessionCount;++i)
		{
			auto session = std::make_shared<Session>();
			session->Initialize(_acceptCallback, _receiveCallback, _disconnectCallback);
			session->Activate(_handle);
			_sessionSet.insert(std::move(session));
		}
		std::string sessionLog = "Session : " + std::to_string(sessionCount) + " Activate Success !!";
		Utility::Log("Network", "NetworkManager", sessionLog);


		Utility::Log("Network", "NetworkManager", "Construct All Success !!");
	}

	void NetworkManager::PrepareSocket()
	{
		for (int index = 0;index < _clientReadyCountMax; ++index)
		{
			SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);
			CreateIoCompletionPort((HANDLE)newSocket, _handle, (ULONG_PTR)socketSharedPtr.get(), _sessionCount);

			_preparedSocketQueue.push(socketSharedPtr);
		}

		std::string log = "���� IOCP ���� : " + std::to_string(_clientReadyCountMax);
		Utility::Log("Network", "NetworkManager", log);
	}

	std::shared_ptr<SOCKET> NetworkManager::GetPreparedSocket()
	{
		std::shared_ptr<SOCKET> prepareSocket = nullptr;
		bool result = _preparedSocketQueue.try_pop(prepareSocket);
		if (result == false)
		{
			PrepareSocket();
			result = _preparedSocketQueue.try_pop(prepareSocket);
		}

		return prepareSocket;
	}

	void NetworkManager::ActivateClient(std::shared_ptr<SOCKET> targetSocket)
	{
		if (targetSocket == nullptr)
		{
			Utility::LogError("Network", "NetworkManager", "Prepare Socket is NULL");
			return;
		}

		std::shared_ptr<Client> clientSharedPtr = std::make_shared<Client>();
		clientSharedPtr->Initialize(targetSocket);
		_activatedClientMap.insert(std::make_pair((ULONG_PTR)targetSocket.get(), clientSharedPtr));
		
		auto overlappedPtr = _overlappedQueue->pop();
		overlappedPtr->Clear();

		clientSharedPtr->AcceptEx(_listenSocket, _acceptExPointer, overlappedPtr);
	}

	void NetworkManager::AcceptCallback(ULONG_PTR targetSocket)
	{
		auto finder = _activatedClientMap.find(targetSocket);
		if (finder == _activatedClientMap.end())
		{
			Utility::LogError("Network", "NetworkManager", "Socket Accpet... Not Find");
			return;
		}

		auto overlappedPtr = _overlappedQueue->pop();
		overlappedPtr->Clear();

		auto client = finder->second;
		client->ReceiveReady(overlappedPtr);

		std::shared_ptr<SOCKET> prepareSocket = GetPreparedSocket();

		if (prepareSocket == nullptr)
		{
			Utility::LogError("Network", "NetworkManager", "Prepare Socket is NULL");
			return;
		}
		ActivateClient(prepareSocket);
	}

	void NetworkManager::ReceiveCallback(ULONG_PTR targetSocket, CustomOverlapped* overlappedPtr)
	{
		if (ProcessMessage == nullptr)
		{
			Utility::LogError("Network", "NetworkManager", "ReadMessage is NULL");
			return;
		}

		auto finder = _activatedClientMap.find(targetSocket);
		if (finder == _activatedClientMap.end())
		{
			Utility::LogError("Network", "NetworkManager", "Socket Recv... Not Find");
			return;
		}

		MessageHeader* receivedHeader = reinterpret_cast<MessageHeader*>(overlappedPtr->Wsabuf[0].buf);
		auto requestBodySize = ntohl(receivedHeader->BodySize);
		auto requestContentsType = ntohl(receivedHeader->ContentsType);

		auto bufferString = std::string(overlappedPtr->Wsabuf[1].buf, requestBodySize);// string ������ ������ ���� buffer�ʱ�ȭ�ÿ��� �������� ��ȭ.
		ProcessMessage(targetSocket, requestContentsType, bufferString);

		overlappedPtr->Clear();
		_overlappedQueue->push(std::move(overlappedPtr));

		auto newOverlappedPtr = _overlappedQueue->pop();
		newOverlappedPtr->Clear();

		auto client = finder->second;
		client->ReceiveReady(newOverlappedPtr);
	}

	void NetworkManager::SendRequest(ULONG_PTR& targetSocket, uint32_t& contentType, std::string& stringBuffer, int& bodySize)
	{
		auto finder = _activatedClientMap.find(targetSocket);
		if (finder == _activatedClientMap.end())
		{
			Utility::Log("Network", "NetworkManager", "Socket Send... Not Find");
			return;
		}
		auto client = finder->second;

		auto newOverlappedPtr = _overlappedQueue->pop();
		newOverlappedPtr->Clear();

		auto responseBodySize = htonl(bodySize);
		auto responseContentType = htonl(contentType);
		MessageHeader messageHeader(responseBodySize, responseContentType);

		client->Send(newOverlappedPtr, messageHeader, stringBuffer, bodySize);
	}

	void NetworkManager::DisconnectCallback(ULONG_PTR targetSocket, int bytesTransferred, int errorCode)
	{
		auto finder = _activatedClientMap.find(targetSocket);
		if (finder == _activatedClientMap.end())
		{
			Utility::LogError("Network", "NetworkManager", "Disconnected Socket... Not Find");
			return;
		}

		UnexpectedDisconnect(targetSocket, errorCode);

		std::shared_ptr<Client> client = finder->second;
		std::shared_ptr<SOCKET> socket = client->ClientSocketPtr;

		shutdown(*socket, SD_BOTH);
		client->Deinitialize();
		Utility::Log("Network", "NetworkManager", "Ŭ���̾�Ʈ ���� ���� Ȯ��.");
		ProcessDisconnect(targetSocket, errorCode);

		// ����ȭ��ü��� �ʿ�.
		_activatedClientMap.unsafe_erase((ULONG_PTR)socket.get());

		_preparedSocketQueue.push(socket);

		std::shared_ptr<SOCKET> prepareSocket = GetPreparedSocket();
		ActivateClient(prepareSocket);
	}

	void NetworkManager::UnexpectedDisconnect(ULONG_PTR targetSocket, int errorCode)
	{
		switch (errorCode)
		{
		case WSAECONNRESET:
			Utility::Log("Network", "NetworkManager", "WSAECONNRESET : ������ ������ ���� ����");
			break;
		case WSAECONNABORTED:
			Utility::Log("Network", "NetworkManager", "WSAECONNABORTED : ���� ������ �ߴܵ�");
			break;
		case WSAENETRESET:
			Utility::Log("Network", "NetworkManager", "WSAENETRESET : ��Ʈ��ũ ������ ����");
			break;
		case WSAETIMEDOUT:
			Utility::Log("Network", "NetworkManager", "WSAETIMEDOUT : ���� ���� ����");
			break;
		case WSAENOTCONN:
			Utility::Log("Network", "NetworkManager", "WSAENOTCONN : ������� ���� ���Ͽ��� recv ȣ��");
			break;
		case WSAESHUTDOWN:
			Utility::Log("Network", "NetworkManager", "WSAESHUTDOWN : shutdown ���� ������ ��û�� �޼���");
			break;
		case ERROR_NETNAME_DELETED:
			Utility::Log("Network", "NetworkManager", "ERROR_NETNAME_DELETED : ������ ��Ʈ��ũ �̸��� �� �̻� ����� �� �����ϴ�.");
			break;
		default:
			Utility::Log("Network", "NetworkManager", "Unexpected error code: " + std::to_string(errorCode));
			break;
		}
	}


	/*
	* - ���� �񵿱������� WSARecv()�� ȣ���Ͽ� IOCP�� ������ ������ ��� ���� ���, shutdown(SD_BOTH) ȣ�� �Ŀ��� �Ϸ���� ���� ���� �۾��� ������ ����� �� �־�.
	* - ������ shutdown(SD_BOTH)���� �� ������ ������ ���ܵǹǷ�, ���� ���ο� WSARecv() ��û�� ������ ���ɼ��� ���� (WSAECONNRESET �Ǵ� WSAESHUTDOWN ���� �߻� ����).


	*/


}