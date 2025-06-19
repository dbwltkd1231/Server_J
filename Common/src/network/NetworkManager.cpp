#pragma once

#include "NetworkManager.h"
#include "Debug.h"
#include "ConstValue.h"


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

	void NetworkManager::Construct()
	{
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
		serverAddr.sin_port = htons(Utility::SERVER_PORT);

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

		_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, Utility::SESSION_COUNT_MAX);
		if (_handle == NULL)
		{
			Utility::LogError("Network", "IOCP", "CreateIoCompletionPort failed");
			closesocket(_listenSocket);
			WSACleanup();
			return;
		}
		Utility::Log("Network", "IOCP", "IOCP Handle Ready");

		if (!CreateIoCompletionPort((HANDLE)_listenSocket, _handle, 0, Utility::SESSION_COUNT_MAX))
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
		_overlappedQueue->Construct(Utility::OVERLAPPED_COUNT_MAX);

		for (int i = 0;i < Utility::OVERLAPPED_COUNT_MAX; ++i)
		{
			auto overlapped = new CustomOverlapped();
			_overlappedQueue->push(std::move(overlapped));
		}
		Utility::Log("Network", "NetworkManager", "OverlappedQueue Ready Success");

		PrepareSocket();

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

		for (int i = 0;i < Utility::SESSION_COUNT_MAX;++i)
		{
			auto session = std::make_shared<Session>();
			session->Initialize(_acceptCallback, _receiveCallback, _disconnectCallback);
			session->Activate(_handle);
			_sessionSet.insert(std::move(session));
		}
		std::string sessionLog = "Session : " + std::to_string(Utility::SESSION_COUNT_MAX) + " Activate Success !!";
		Utility::Log("Network", "NetworkManager", sessionLog);


		Utility::Log("Network", "NetworkManager", "Construct All Success !!");
	}

	void NetworkManager::PrepareSocket()
	{
		for (int index = 0;index < Utility::SOCKET_ONETIME_PREPARE_COUNT; ++index)
		{
			SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);
			CreateIoCompletionPort((HANDLE)newSocket, _handle, (ULONG_PTR)socketSharedPtr.get(), Utility::SESSION_COUNT_MAX);

			_preparedSocketQueue.push(socketSharedPtr);
		}

		std::string log = "SOCKET READY : " + std::to_string(Utility::SOCKET_ONETIME_PREPARE_COUNT);
		Utility::LogError("Network", "NetworkManager", log);
	}

	void NetworkManager::ActivateClient()
	{
		int capacity = Utility::CLIENT_ACTIVATE_COUNT_MAX - _activatedClientMap.size();
		if (capacity < 1)
		{
			Utility::LogError("Network", "NetworkManager", "수용 가능한 클라이언트 초과 !!");
			return;
		}

		std::shared_ptr<SOCKET> newSocket;
		bool result = _preparedSocketQueue.try_pop(newSocket);
		if (!result)
		{
			Utility::LogError("Network", "NetworkManager", "소켓부족 -> 생성요청");
			PrepareSocket();
			return;
		}

		std::shared_ptr<Client> clientSharedPtr = std::make_shared<Client>();
		clientSharedPtr->Initialize(newSocket);

		auto overlappedPtr = _overlappedQueue->pop();
		overlappedPtr->Clear();

		clientSharedPtr->AcceptEx(_listenSocket, _acceptExPointer, overlappedPtr);

		_activatedClientMap.insert(std::make_pair((ULONG_PTR)newSocket.get(), clientSharedPtr));
		

		Utility::Log("Network", "NetworkManager", "Client Accept Ready Success");

		if (_preparedSocketQueue.unsafe_size() < 1)//괜찮을까?
		{
			PrepareSocket();
		}
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

		Utility::Log("Network", "NetworkManager", "Socket Accpet Success And Recv Ready Success !!");
	}

	void NetworkManager::ReceiveCallback(ULONG_PTR targetSocket, CustomOverlapped* overlappedPtr)
	{
		if (ReadMessage == nullptr)
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

		auto bufferString = std::string(overlappedPtr->Wsabuf[1].buf, requestBodySize);// string 값복사 전달을 통해 buffer초기화시에도 안정성을 강화.
		ReadMessage(targetSocket, requestContentsType, bufferString);

		overlappedPtr->Clear();
		_overlappedQueue->push(std::move(overlappedPtr));

		auto newOverlappedPtr = _overlappedQueue->pop();
		newOverlappedPtr->Clear();

		auto client = finder->second;
		client->ReceiveReady(newOverlappedPtr);
	}

	void NetworkManager::SendRequest(ULONG_PTR targetSocket, uint32_t contentType, int bodySize, std::string stringBuffer)
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

		// 동기화객체사용 필요.
		_activatedClientMap.unsafe_erase((ULONG_PTR)socket.get());

		_preparedSocketQueue.push(socket);

		ActivateClient();
	}

	void NetworkManager::UnexpectedDisconnect(ULONG_PTR targetSocket, int errorCode)
	{
		switch (errorCode)
		{
		case WSAECONNRESET:
			Utility::LogError("Network", "NetworkManager", "상대방이 강제로 연결 종료");
			break;
		case WSAECONNABORTED:
			Utility::LogError("Network", "NetworkManager", "서버 연결이 중단됨");
			break;
		case WSAENETRESET:
			Utility::LogError("Network", "NetworkManager", "네트워크 연결이 끊김");
			break;
		case WSAETIMEDOUT:
			Utility::LogError("Network", "NetworkManager", "상대방 응답 없음");
			break;
		case WSAENOTCONN:
			Utility::LogError("Network", "NetworkManager", "연결되지 않은 소켓에서 recv 호출");
			break;
		case WSAESHUTDOWN:
			Utility::LogError("Network", "NetworkManager", "shutdown 상태 소켓이 요청한 메세지");
			break;
		default:
			break;
		}
	}
	/*
	* - 만약 비동기적으로 WSARecv()를 호출하여 IOCP가 데이터 수신을 대기 중인 경우, shutdown(SD_BOTH) 호출 후에도 완료되지 않은 수신 작업은 여전히 진행될 수 있어.
	* - 하지만 shutdown(SD_BOTH)으로 새 데이터 수신이 차단되므로, 이후 새로운 WSARecv() 요청은 실패할 가능성이 높아 (WSAECONNRESET 또는 WSAESHUTDOWN 오류 발생 가능).


	*/


}