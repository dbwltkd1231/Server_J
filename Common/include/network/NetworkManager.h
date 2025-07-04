#pragma once
#include <memory>
#include <functional>
#include <set>

#define NOMINMAX
#include <winsock2.h>
#include <MSWSock.h>
#include <windows.h>

#include "oneapi/tbb/concurrent_map.h"
#include "oneapi/tbb/concurrent_queue.h"

#include "Client.h"
#include "CustomOverlapped.h"
#include "Session.h"
#include "LockFreeCircleQueue.h"

namespace Network
{
	class NetworkManager
	{
	public:
		NetworkManager();
		~NetworkManager();

	private:
		SOCKET _listenSocket;
		HANDLE _handle = INVALID_HANDLE_VALUE;
		LPFN_ACCEPTEX _acceptExPointer = nullptr;
		
	private:
		std::shared_ptr<Utility::LockFreeCircleQueue<CustomOverlapped*>> _overlappedQueue;
		tbb::concurrent_map<ULONG_PTR, std::shared_ptr<Client>> _activatedClientMap;

		std::set<std::shared_ptr<Session>> _sessionSet;

		//session -> networkmanager callback
	private:
		std::function<void(CustomOverlapped*, ULONG_PTR)> _acceptProcess;
		std::function<void(CustomOverlapped*, ULONG_PTR)> _receiveProcess;
		std::function<void(CustomOverlapped*, ULONG_PTR socket, int bytesTransferred, int errorCode)> _disconnectProcess;
		std::function<void(CustomOverlapped*)> _sendProcess;
		//networkmanager -> auth,lobbymanager callback
	public:
		std::function<void(ULONG_PTR&)> AcceptCallback;
		std::function<void(ULONG_PTR&, uint32_t, std::string)> ReceiveCallback;
		std::function<void(ULONG_PTR&, int)> DisconnectCallback;

	public:
		void Construct(int serverPort, int sessionCount, int overlappedCount, int clientReadyCountMax); //IOCP handle 생성, 컨테이너 생성, 송수신콜백연결, Session 생성 등 초기세팅목적.
		void ActivateClient(std::shared_ptr<SOCKET> targetSocket); // 클라이언트 Acceptex호출 목적.

	private:
		int _clientReadyCountMax;
		int _sessionCount;

	private:
		tbb::concurrent_queue<std::shared_ptr<SOCKET>> _preparedSocketQueue;
	public:
		void PrepareSocket();// 준비된 소켓이 0개일때 소켓을 생성하는 코드.
		std::shared_ptr<SOCKET> GetPreparedSocket();

	private:
		//Session으로 부터 호출되는 메세시처리 함수들.
		void Accept(CustomOverlapped* overlappedPtr, ULONG_PTR targetSocket);
		void Receive(CustomOverlapped* overlappedPtr, ULONG_PTR targetSocket);
		void Disconnect(CustomOverlapped* overlappedPtr, ULONG_PTR targetSocket, int bytesTransferred, int errorCode);
		void Send(CustomOverlapped* overlappedPtr);
	public:
		void DisconnectRequest(ULONG_PTR targetSocket);
		void SendRequest(ULONG_PTR& targetSocket, uint32_t& contentType, std::string& stringBuffer, int& bodySize); // auth,lobby logic에서 메세지 송신시 콜백되는 함수.

	private:
		void UnexpectedDisconnect(ULONG_PTR targetSocket, int errorCode);//비정상접속종료 처리.

	};
}