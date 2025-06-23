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

	private:
		std::function<void(ULONG_PTR)> _acceptCallback;
		std::function<void(ULONG_PTR, CustomOverlapped*)> _receiveCallback;
		std::function<void(ULONG_PTR socket, int bytesTransferred, int errorCode)> _disconnectCallback;

	public:
		std::function<void(ULONG_PTR&, uint32_t, std::string)> ReadMessage;

	public:
		void Construct(); //IOCP handle 생성, 컨테이너 생성, 송수신콜백연결, Session 생성 등 초기세팅목적.
		void ActivateClient(std::shared_ptr<SOCKET> targetSocket); // 클라이언트 Acceptex호출 목적.

	private:
		tbb::concurrent_queue<std::shared_ptr<SOCKET>> _preparedSocketQueue;
	public:
		void PrepareSocket(int count);// 준비된 소켓이 0개일때 소켓을 생성하는 코드.
		std::shared_ptr<SOCKET> GetPreparedSocket();

	private:
		//Session으로 부터 호출되는 메세시처리 함수들.
		void AcceptCallback(ULONG_PTR targetSocket);
		void ReceiveCallback(ULONG_PTR targetSocket, CustomOverlapped* overlappedPtr);
		void DisconnectCallback(ULONG_PTR targetSocket, int bytesTransferred, int errorCode);

	public:
		void SendRequest(ULONG_PTR& targetSocket, uint32_t& contentType, std::string& stringBuffer, int& bodySize); // auth,lobby logic에서 메세지 송신시 콜백되는 함수.

	private:
		void UnexpectedDisconnect(ULONG_PTR targetSocket, int errorCode);//비정상접속종료 처리.

	};
}