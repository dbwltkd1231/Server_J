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
		void Construct(); //IOCP handle ����, �����̳� ����, �ۼ����ݹ鿬��, Session ���� �� �ʱ⼼�ø���.
		void ActivateClient(std::shared_ptr<SOCKET> targetSocket); // Ŭ���̾�Ʈ Acceptexȣ�� ����.

	private:
		tbb::concurrent_queue<std::shared_ptr<SOCKET>> _preparedSocketQueue;
	public:
		void PrepareSocket(int count);// �غ�� ������ 0���϶� ������ �����ϴ� �ڵ�.
		std::shared_ptr<SOCKET> GetPreparedSocket();

	private:
		//Session���� ���� ȣ��Ǵ� �޼���ó�� �Լ���.
		void AcceptCallback(ULONG_PTR targetSocket);
		void ReceiveCallback(ULONG_PTR targetSocket, CustomOverlapped* overlappedPtr);
		void DisconnectCallback(ULONG_PTR targetSocket, int bytesTransferred, int errorCode);

	public:
		void SendRequest(ULONG_PTR& targetSocket, uint32_t& contentType, std::string& stringBuffer, int& bodySize); // auth,lobby logic���� �޼��� �۽Ž� �ݹ�Ǵ� �Լ�.

	private:
		void UnexpectedDisconnect(ULONG_PTR targetSocket, int errorCode);//�������������� ó��.

	};
}