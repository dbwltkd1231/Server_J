#pragma once
#include "game/User.h"

#include "../utility/Debug.h"

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

	void User::Initialize(std::shared_ptr<SOCKET> targetSocket)
	{
		_client.Initialize(targetSocket);
	}

//void User::Initialize(int64_t accountNumber, std::string userID, int money)
//{
//	_accountNumber = accountNumber;
//	_userID = userID;
//	_money = money;
//	Utility::Log("Game", "User", "Initialize: " + userID + " with Account Number: " + std::to_string(accountNumber));
//}
}