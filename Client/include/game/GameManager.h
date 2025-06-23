#pragma once
#include <string>
#include "network/NetworkManager.h"

namespace Game
{
	class GameManager
	{
	public:
		GameManager();
		~GameManager();

	public:
		void Initialize(std::string ip, int port);
		void Process(int threadCount);
	private:
		Network::NetworkManager _networkManager;

	private:
		Game::User _user; // �ϴ� �ϳ��� ������ ����
	};
}