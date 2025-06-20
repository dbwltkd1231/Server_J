#pragma once
#include <memory>

#include "network/Client.h"
#include "oneapi/tbb/concurrent_map.h"
#include "../utility/LockFreeCircleQueue.h"

namespace Client
{
	class ClientManager
	{
	public:
		ClientManager();
		~ClientManager();

	private:
		HANDLE _iocpHandle;
		bool _connected = false;
	public:
		void Initialize(std::string ip, int port, int clientCount, int threadCount);
		void Process(int threadCount);

	private:
		void Work();
	private:
		tbb::concurrent_map<ULONG_PTR, std::shared_ptr<Network::Client>> _clientMap;
		Utility::LockFreeCircleQueue<Network::CustomOverlapped*> _overlappedQueue;
	};
}
