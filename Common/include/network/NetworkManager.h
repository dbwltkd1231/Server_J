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
		std::function<void(ULONG_PTR)> _acceptProcess;
		std::function<void(ULONG_PTR, CustomOverlapped*)> _receiveProcess;
		std::function<void(ULONG_PTR socket, int bytesTransferred, int errorCode)> _disconnectProcess;

		//networkmanager -> auth,lobbymanager callback
	public:
		std::function<void(ULONG_PTR&)> AcceptCallback;
		std::function<void(ULONG_PTR&, uint32_t, std::string)> ReceiveCallback;
		std::function<void(ULONG_PTR&, int)> DisconnectCallback;

	public:
		void Construct(int serverPort, int sessionCount, int overlappedCount, int clientReadyCountMax); //IOCP handle ����, �����̳� ����, �ۼ����ݹ鿬��, Session ���� �� �ʱ⼼�ø���.
		void ActivateClient(std::shared_ptr<SOCKET> targetSocket); // Ŭ���̾�Ʈ Acceptexȣ�� ����.

	private:
		int _clientReadyCountMax;
		int _sessionCount;

	private:
		tbb::concurrent_queue<std::shared_ptr<SOCKET>> _preparedSocketQueue;
	public:
		void PrepareSocket();// �غ�� ������ 0���϶� ������ �����ϴ� �ڵ�.
		std::shared_ptr<SOCKET> GetPreparedSocket();

	private:
		//Session���� ���� ȣ��Ǵ� �޼���ó�� �Լ���.
		void Accept(ULONG_PTR targetSocket);
		void Receive(ULONG_PTR targetSocket, CustomOverlapped* overlappedPtr);
		void Disconnect(ULONG_PTR targetSocket, int bytesTransferred, int errorCode);

	public:
		void SendRequest(ULONG_PTR& targetSocket, uint32_t& contentType, std::string& stringBuffer, int& bodySize); // auth,lobby logic���� �޼��� �۽Ž� �ݹ�Ǵ� �Լ�.

	private:
		void UnexpectedDisconnect(ULONG_PTR targetSocket, int errorCode);//�������������� ó��.

	};
}