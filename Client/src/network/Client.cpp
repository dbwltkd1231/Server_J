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

	void Client::Initialize()
	{
		SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		auto socketSharedPtr = std::make_shared<SOCKET>(newSocket);

		ClientSocketPtr = socketSharedPtr;
		Utility::Log("Network", "Client", "Initialize");
	}

	void Client::Deinitialize()
	{
		if (ClientSocketPtr && *ClientSocketPtr != INVALID_SOCKET)
		{
			// 송수신 모두 종료 시도
			shutdown(*ClientSocketPtr, SD_BOTH);

			// 소켓 닫기
			closesocket(*ClientSocketPtr);
			*ClientSocketPtr = INVALID_SOCKET;
		}

		// 안전하게 nullptr로 설정
		ClientSocketPtr = nullptr;

		Utility::Log("Network", "Client", "Deinitialize");
	}

	std::shared_ptr<SOCKET> Client::GetSocket()
	{
		return ClientSocketPtr;
	}

	void Client::ConnectEx(LPFN_CONNECTEX& connectEx, sockaddr_in serverAddr, Network::CustomOverlapped& overlapped)
	{
		// 소켓이 유효한지 먼저 확인
		if (ClientSocketPtr == nullptr || *ClientSocketPtr == INVALID_SOCKET) 
		{
			Utility::LogError("Network", "Client", "ConnectEx 실패: 잘못된 소켓 포인터");
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
				Utility::LogError("Network", "Client", "ConnectEx 실패! 오류 코드: " + std::to_string(errorCode));

				// 소켓을 안전하게 닫고 정리
				if (*ClientSocketPtr != INVALID_SOCKET) 
				{
					closesocket(*ClientSocketPtr);
					*ClientSocketPtr = INVALID_SOCKET;
				}
				WSACleanup();
				return;
			}
		}

		Utility::Log("Network", "Client", "ConnectEx 시작됨, WSA_IO_PENDING 상태");
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
			log = "WSARecv 실패! 오류 코드: " + std::to_string(errorCode);
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
			std::string log = "WSASend 실패! 오류 코드: " + std::to_string(errorCode);
			Utility::Log("Network", "Client", log);
			return;
		}

		Utility::Log("Network", "Client", "클라이언트 WSASend 호출");
	}
} 