#pragma once
#include <string>
#include <functional>

#include "network/NetworkManager.h"

#include "oneapi/tbb/concurrent_map.h"

#include "../utility/LockFreeCircleQueue.h"
namespace Game
{
	class GameManager
	{
	public:
		GameManager();
		~GameManager();

	private:
		Network::NetworkManager _networkManager;

	public:
		void Initialize(std::string ip, int port, int clientCount);
		void Process(int threadCount);

	private:
		void AcceptCallback(Network::ServerType targetServer, ULONG_PTR targetSocket);
		void ReceiveCallback(Network::ServerType targetServer, ULONG_PTR targetSocket, Network::CustomOverlapped* overlappedPtr);
		void DisconnectCallback(Network::ServerType targetServer, ULONG_PTR targetSocket, int bytesTransferred, int errorCode);

	private:
		std::function<void(Network::ServerType, ULONG_PTR)> _authCallback;
		std::function<void(Network::ServerType, ULONG_PTR, Network::CustomOverlapped*)> _receiveCallback;
		std::function<void(Network::ServerType, ULONG_PTR, int, int)> _disconnectCallback;

	private:
		std::shared_ptr<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>> _overlappedQueue;
		std::shared_ptr<tbb::concurrent_map<Network::ServerType, ULONG_PTR>> _serverSocketMap;
		std::shared_ptr<tbb::concurrent_map<ULONG_PTR, Game::User>> _socketUserMap;
	};
}