#pragma once
#include <string>
#include <memory>
#include <thread>

#include "network/Client.h"

#include "oneapi/tbb/concurrent_map.h"


namespace Network
{

	enum ServerType
	{
		Default = -1,
		Auth,
		Lobby
	};


	class NetManagerModule 
	{
	public:
		NetManagerModule();
		~NetManagerModule();

	private:
		ServerType _serverType;

	public:
		void Initialize(std::string ip, int port, ServerType serverType);
		void CallbackSetting(
			std::function<void(Network::ServerType, ULONG_PTR)> acceptCallback,
			std::function<void(Network::ServerType, ULONG_PTR, CustomOverlapped*)> receiveCallback,
			std::function<void(Network::ServerType, ULONG_PTR socket, int bytesTransferred, int errorCode)> disconnectCallback
		);
		bool Connect(std::shared_ptr<Network::Client> targetClient, std::shared_ptr<SOCKET> targetSocket, DWORD concurrentThread, Network::CustomOverlapped* overlappedPtr);
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

	private:
		std::function<void(Network::ServerType, ULONG_PTR)> _acceptCallback;
		std::function<void(Network::ServerType, ULONG_PTR, int, int)> _disconnectCallback;
		std::function<void(Network::ServerType, ULONG_PTR, CustomOverlapped*)> _receiveCallback;

	private:
		bool _isOn;
	};
}