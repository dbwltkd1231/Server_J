#pragma once
#include "game/User.h"
#include "../utility/Debug.h"
#include "game/ConstValue.h"

namespace Game
{
	User::User()
	{
		// 초기화 코드
	}

	User::~User()
	{
		// 정리 코드
	}

	void User::Initialize(std::shared_ptr<Network::Client> client)
	{
		ClientPtr = client;
	}

	void User::Deinitialize()
	{
		ClientPtr->Deinitialize();

		ClientPtr = nullptr;
	}

	void User::SetAccountData(std::string authToken, int64_t accountNumber, std::string userId)
	{
		_authToken = authToken;
		_accountNumber = accountNumber;
		_userID = userId;
	}

	void User::RequestConnect(Network::CustomOverlapped* sendOverlappedPtr)
	{
		uint32_t contentsType = 0;
		std::string stringBuffer = "";
		int bodySize = 0;

		int clientNumber = Game::ConstValue::GetInstance().CurrentClinetIndex.fetch_add(1, std::memory_order_relaxed);
		std::string uid = Game::ConstValue::GetInstance().TestUID + std::to_string(clientNumber);

		Common::Auth::CreateRequestConnect(uid, contentsType, stringBuffer, bodySize);
		Network::MessageHeader newHeader(htonl(bodySize), htonl(contentsType));
		ClientPtr->Send(sendOverlappedPtr, newHeader, stringBuffer, bodySize);
	}

	void User::RequestLogIn(Network::CustomOverlapped* sendOverlappedPtr)
	{
		Common::Lobby::PacketOutput output;
		Common::Lobby::CreateRequestLogIn(_accountNumber, _authToken, output);
		Network::MessageHeader newHeader(htonl(output.BodySize), htonl(output.ContentsType));

		ClientPtr->Send(sendOverlappedPtr, newHeader, output.Buffer, output.BodySize);
	}


}