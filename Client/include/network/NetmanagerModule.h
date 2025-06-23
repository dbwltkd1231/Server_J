#pragma once
#include <string>
#include <memory>
#include <thread>

#include "network/Client.h"

#include "oneapi/tbb/concurrent_map.h"
#include "../utility/LockFreeCircleQueue.h"


namespace Network
{
	class NetManagerModule 
	{
	public:
		NetManagerModule();
		~NetManagerModule();

	public:
		void Initialize(std::string ip, int port);
		bool Connect(std::shared_ptr<Network::Client> targetClient, std::shared_ptr<SOCKET> targetSocket, DWORD concurrentThread);
		void Process(int threadCount);
		void Work();

	private:
		HANDLE _iocpHandle;
		WSADATA wsaData;
		LPFN_CONNECTEX connectEx = NULL;
		GUID connectExGuid = WSAID_CONNECTEX;
		sockaddr_in serverAddr;
		sockaddr_in localAddr;

	private:
		tbb::concurrent_map<ULONG_PTR, std::shared_ptr<Network::Client>> _clientMap;
		std::shared_ptr<Utility::LockFreeCircleQueue<CustomOverlapped*>> _overlappedQueue;
	private:
		bool _isOn;
	};
}