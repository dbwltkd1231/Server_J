#pragma once
#include "game/User.h"
#include "../utility/Debug.h"
#include "game/ConstValue.h"

namespace Game
{
	User::User()
	{
		// �ʱ�ȭ �ڵ�
	}

	User::~User()
	{
		// ���� �ڵ�
	}

	void User::Initialize(std::shared_ptr<Network::Client> client, Network::CustomOverlapped* sendOverlappedPtr)
	{
		_client = client;

		uint32_t contentsType = 0;
		std::string stringBuffer = "";
		int bodySize = 0;

		int clientNumber = Game::ConstValue::GetInstance().CurrentClinetIndex.fetch_add(1, std::memory_order_relaxed);
		std::string uid = Game::ConstValue::GetInstance().TestUID + std::to_string(clientNumber);

		Game::Protocol::CreateRequestConnect(uid, contentsType, stringBuffer, bodySize);
		Network::MessageHeader newHeader(htonl(bodySize), htonl(contentsType));
		_client->Send(sendOverlappedPtr, newHeader, stringBuffer, bodySize);
	}
}