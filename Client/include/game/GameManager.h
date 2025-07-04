#pragma once
#include <string>
#include <functional>
#include "game/User.h"
#include "oneapi/tbb/concurrent_map.h"
#include "game/ConstValue.h"
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
		void Initialize(int clientCount);
		void Process(int threadCount);

	private:
		void AcceptCallback(Network::ServerType& targetServer, std::shared_ptr<Network::Client> client, Network::CustomOverlapped* overlappedPtr);
		void ReceiveCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, Network::CustomOverlapped* overlappedPtr);
		void DisconnectCallback(Network::ServerType& targetServer, ULONG_PTR& targetSocket, int bytesTransferred, int errorCode, Network::CustomOverlapped* overlappedPtr);
		void SendCallback(Network::CustomOverlapped* overlappedPtr);

	private:
		void ReadAuthMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string& stringValue);
		void ReadLobbyMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string& stringValue);
		
	private:
		std::function<void(Network::ServerType&, std::shared_ptr<Network::Client>, Network::CustomOverlapped*)> _acceptCallback;
		std::function<void(Network::ServerType&, ULONG_PTR&, Network::CustomOverlapped*)> _receiveCallback;
		std::function<void(Network::ServerType&, ULONG_PTR&, int, int, Network::CustomOverlapped*)> _disconnectCallback;
		std::function<void(Network::CustomOverlapped*)> _sendCallback;

	private:
		std::shared_ptr<Utility::LockFreeCircleQueue<Network::CustomOverlapped*>> _overlappedQueue;
		std::shared_ptr<tbb::concurrent_map<ULONG_PTR, std::shared_ptr<Game::User>>> _socketUserMap;

	private:
		bool _isOn;
	};
}