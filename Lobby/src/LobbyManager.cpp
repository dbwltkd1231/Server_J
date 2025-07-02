#pragma once

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"

#include "LobbyManager.h"
#include "ConstValue.h"
#include "../lobby/LobbyProtocol.h"
#include "../lobby/DatabaseProtocol.h"
#include <LobbyProcedureCreator.h>



namespace Lobby
{
	LobbyManager::LobbyManager()
	{
		_redis = nullptr;
	}

	LobbyManager::~LobbyManager()
	{
		_redis = nullptr;
	}

	void LobbyManager::Initialize()
	{
		_networkManager.Construct(Lobby::ConstValue::GetInstance().StartPort,
			Lobby::ConstValue::GetInstance().SessionCountMax,
			Lobby::ConstValue::GetInstance().OverlappedCountMax,
			Lobby::ConstValue::GetInstance().ConnectReadyClientCountMax);

		_networkManager.PrepareSocket();

		for (int i = 0;i < Lobby::ConstValue::GetInstance().ConnectReadyClientCountMax;++i)
		{
			std::shared_ptr<SOCKET> targetSocket = _networkManager.GetPreparedSocket();
			_networkManager.ActivateClient(targetSocket);
		}

		std::string clientLog = "Client : " + std::to_string(Lobby::ConstValue::GetInstance().ConnectReadyClientCountMax) + " Activate Success !!";
		Utility::Log("Lobby", "LobbyManager", clientLog);


		_networkManager.AcceptCallback = std::function<void(ULONG_PTR&)>
			(
				[this]
				(ULONG_PTR& targetSocket)
				{
					this->ProcessAccept(targetSocket);
				}
			);

		_networkManager.ReceiveCallback = std::function<void(ULONG_PTR&, uint32_t, std::string)>
			(
				[this]
				(ULONG_PTR& targetSocket, uint32_t contentsType, std::string buffer)
				{
					this->ReadMessage(targetSocket, contentsType, buffer);
				}
			);

		_networkManager.DisconnectCallback = std::function<void(ULONG_PTR&, int)>
			(
				[this]
				(ULONG_PTR& targetSocket, int errorCode)
				{
					this->ProcessDisconnect(targetSocket, errorCode);
				}
			);


		_callbackProcedureResult = std::function<void(ULONG_PTR, uint32_t, SQLHSTMT)>
			(
				[this]
				(ULONG_PTR socketPtr, uint32_t contentsType, SQLHSTMT hstmt)
				{
					this->SendQueryResult(socketPtr, contentsType, hstmt);
				}
			);
	}

	void LobbyManager::ConnectDatabase(std::string userDatabaseName, std::string userSqlServerAddress, std::string gameDatabaseName, std::string gameSqlServerAddress)
	{
		_userDatabaseWorker.Initialize(userDatabaseName, userSqlServerAddress, _callbackProcedureResult);
		_userDatabaseWorker.Activate(true);
		Utility::Log("Lobby", "LobbyManager", "UserDatabase Worker Process..");

		_gameDatabaseWorker.Initialize(gameDatabaseName, gameSqlServerAddress, _callbackProcedureResult);
		_gameDatabaseWorker.Activate(true);
		Utility::Log("Lobby", "LobbyManager", "GameDatabase Worker Process..");
	}

	void LobbyManager::ConnectRedis(std::string ip, int redisPort)
	{
		_redis = redisConnect(ip.c_str(), redisPort);
		if (_redis == NULL || _redis->err)
		{
			Utility::Log("Lobby", "LobbyManager", "Redis Connect Fail");
			return;
		}
		Utility::Log("Lobby", "LobbyManager", "Redis Connect Success");
	}

	void LobbyManager::ProcessAccept(ULONG_PTR& targetSocket)
	{
		Utility::Log("Lobby", "LobbyManager", "ProcessAccept");
	}

	void LobbyManager::ProcessDisconnect(ULONG_PTR& targetSocket, int errorCode)
	{
		auto finder = _socketAccountNumber.find(targetSocket);

		//LogOut처리.
		if (finder != _socketAccountNumber.end())
		{
			auto accountNumber = finder->second;
			auto userLogOutTask = Common::Lobby::CreateQuerryUserLogOut(targetSocket, accountNumber, -1);
			_userDatabaseWorker.Enqueue(std::move(userLogOutTask));

			_socketAccountNumber.unsafe_erase(targetSocket);
			Utility::Log("Lobby", "LobbyManager", std::to_string(accountNumber)+" 로그아웃 완료.");
		}
	}

	void LobbyManager::ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();
	
		Database::Task task;

		switch (messageType)
		{
			case protocol::MessageContent_REQUEST_LOGIN:
			{
				auto requestConnect = flatbuffers::GetRoot<protocol::REQUEST_LOGIN>(buffer);
				long accountNumber = requestConnect->account_number();
				std::string authToken = requestConnect->auth_token()->str();

				//authToken 확인 필요 !!

				task = Common::Lobby::CreateQuerryUserLogIn(targetSocket, accountNumber, protocol::MessageContent_RESPONSE_LOGIN);

				break;
			}
		}

		if (task.DatabaseName == Database::DatabaseType::User)
		{
			_userDatabaseWorker.Enqueue(std::move(task));
		}
		else if (task.DatabaseName == Database::DatabaseType::Game)
		{

		}
		else
		{

		}
	}

	void LobbyManager::SendQueryResult(ULONG_PTR targetSocket, uint32_t contentsType, SQLHSTMT& hstmt)
	{
		if (contentsType == -1)
			return;

		auto contentsTypeOffset = static_cast<protocol::MessageContent> (contentsType);
		Common::Lobby::PacketOutput output;

		switch (contentsType)
		{
			case protocol::MessageContent_RESPONSE_LOGIN:
			{
				Common::Protocol::ResultUserLogIn userLoginResult;
				userLoginResult.SetProcedureResult(hstmt);

				Common::Lobby::CreateResponseLogIn(userLoginResult.Detail, userLoginResult.Success, output);

				//로비 상태 저장

				if (userLoginResult.Success)
				{
					auto accountDataTask = Common::Lobby::CreateQuerryAccountData(targetSocket, userLoginResult.AccountNumber, protocol::MessageContent_NOTICE_ACCOUNT);
					_userDatabaseWorker.Enqueue(std::move(accountDataTask));
					_socketAccountNumber.insert({ targetSocket, userLoginResult.AccountNumber });
				}

				break;
			}
			case protocol::MessageContent_NOTICE_ACCOUNT:
			{
				Common::Protocol::ResultGetAccountData getAccountDataResult;
				getAccountDataResult.SetProcedureResult(hstmt);

				Common::Lobby::NoticeAccount(getAccountDataResult.AccountUID, getAccountDataResult.GameMoney, getAccountDataResult.GameMoneyRank, getAccountDataResult.InventoryCapacity, output);
				break;
			}
			default:
			{
				break;
			}
		}

		_networkManager.SendRequest(targetSocket, contentsType, output.Buffer, output.BodySize);
	}

	// main에서 JOIN으로 호출하자
	void LobbyManager::MainThread()
	{
		while (true)
		{
			//접속 후 60초 이내 인증 처리되지 않은 클라이언트 접속 끊는 기능 추가

			//유저는 로비 서버 접속 후 60초마다 아이템을 1~3개 랜덤으로 지급받는다. -> 둘다처리할수있는 타이머객체가 하나있으면 좋으려나? 만들수있을까?
		}
	}
}

/*
* 타이머 기반 이벤트 큐 또는 타임휠 구조
- 방식: 인증 대기 중인 클라이언트를 타임휠 또는 타임아웃 큐에 등록해두고, 메인 로직의 이벤트 루프에서 주기적으로 체크
- 장점:
- 스레드를 많이 만들지 않아도 됨
- 클라이언트 수가 많아도 퍼포먼스 유지가 쉬움

*/