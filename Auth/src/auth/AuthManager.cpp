#pragma once

#include "auth/ConstValue.h"
#include "auth/AuthManager.h"
#include "../game/NetworkProtocol.h"
#include "../game/DatabaseProtocol.h"
#include "../game/BasicData.h"
#include "../utility/Debug.h"


namespace Auth
{
	AuthManager::AuthManager()
	{

	}

	AuthManager::~AuthManager()
	{

	}

	void AuthManager::Initialize()
	{
		_networkManager.Construct(Auth::ConstValue::GetInstance().ServerPort,
			Auth::ConstValue::GetInstance().SessionCountMax,
			Auth::ConstValue::GetInstance().OverlappedCountMax,
			Auth::ConstValue::GetInstance().ConnectReadyClientCountMax);
		_networkManager.PrepareSocket();

		for (int i = 0;i < Auth::ConstValue::GetInstance().ConnectReadyClientCountMax;++i)
		{
			std::shared_ptr<SOCKET> targetSocket = _networkManager.GetPreparedSocket();
			_networkManager.ActivateClient(targetSocket);
		}

		std::string clientLog = "Client : " + std::to_string(Auth::ConstValue::GetInstance().ConnectReadyClientCountMax) + " Activate Success !!";
		Utility::Log("Auth", "AuthManager", clientLog);

		_networkManager.ReadMessage = std::function<void(ULONG_PTR&, uint32_t, std::string)>
			(
				[this]
				(ULONG_PTR& targetSocket, uint32_t contentsType, std::string buffer)
				{
					this->ReadMessage(targetSocket, contentsType, buffer);
				}
			);

		_databaseCallback = std::function<void(ULONG_PTR, uint32_t, SQLHSTMT)>
			(
				[this]
				(ULONG_PTR socketPtr, uint32_t contentsType, SQLHSTMT hstmt)
				{
					this->DatabaseCallback(socketPtr, contentsType, hstmt);
				}
			);
	}

	void AuthManager::ConnectDatabase(std::string databaseName, std::string sqlServerAddress)
	{
		_userDatabaseWorker.Initialize(databaseName, sqlServerAddress, _databaseCallback);
		_userDatabaseWorker.Activate(true);
		Utility::Log("Auth", "AuthManager", "UserDatabase Worker Process..");
	}

	void AuthManager::ConnectRedis(std::string ip, int redisPort)
	{
		_redis = redisConnect(ip.c_str(), redisPort);
		if (_redis == NULL || _redis->err)
		{
			Utility::Log("Auth", "AuthManager", "Redis Connect Fail");
			return;
		}
		Utility::Log("Auth", "AuthManager", "Redis Connect Success");
	}

	void AuthManager::ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();

		Database::Task task;

		switch (messageType)
		{
			case protocol::MessageContent_REQUEST_CONNECT:
			{
				auto requestConnect = flatbuffers::GetRoot<protocol::REQUEST_CONNECT>(buffer);

				std::string loginId = requestConnect->login_id()->str();

				task = Game::CreateQuerryAccountCheck(targetSocket, loginId);
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

	void AuthManager::DatabaseCallback(ULONG_PTR targetSocket, uint32_t contentsType, SQLHSTMT& hstmt)
	{
		std::shared_ptr<Game::BasicData> result = Game::GetSqlData(targetSocket, contentsType, hstmt);

		auto contentsTypeOffset = static_cast<protocol::MessageContent> (result->ContentsType);
		std::string stringBuffer;
		int bodySize = 0;
		switch (contentsType)
		{
			case protocol::MessageContent_REQUEST_CONNECT:
			{
				auto requestConnectData = std::static_pointer_cast<Game::RequestConnectData>(result);

				//TOKEN처리
				CheckLobbyServerState();

				Game::Protocol::CreateResponseConnect(requestConnectData->UID, requestConnectData->IsNew, "TOKEN", Auth::ConstValue::GetInstance().ServerPort, contentsType, stringBuffer, bodySize);
				break;
			}
			default:
				break;
		}

		_networkManager.SendRequest(targetSocket, contentsType, stringBuffer, bodySize);
		
		//success -> 토큰발급
	}

	void AuthManager::CheckLobbyServerState()
	{
		unsigned long long cursor = 0;
		do {
			redisReply* reply = (redisReply*)redisCommand(_redis, "SCAN %llu MATCH Lobby:* COUNT 10", cursor);

			if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) // key, element구조이기때문에 최소2개가있어야 데이터1개를 조회할수있다.
			{
				std::cerr << "Redis SCAN 실패!" << std::endl;
				return;
			}

			cursor = std::stoi(reply->element[0]->str); // 다음 SCAN 커서 업데이트
			redisReply* keyList = reply->element[1];

			if (keyList->type == REDIS_REPLY_ARRAY)
			{
				for (size_t i = 0; i < keyList->elements; i++)
				{
					std::string key = keyList->element[i]->str;

					redisReply* replyData = (redisReply*)redisCommand(_redis, "HGETALL %s", key.c_str());
					if (replyData && replyData->type == REDIS_REPLY_ARRAY)
					{
						int connected = -1, capacity = -1, port = -1;

						for (size_t j = 0; j < replyData->elements; j += 2)
						{
							std::string field = replyData->element[j]->str;
							std::string value = replyData->element[j + 1]->str;

							if (field == "connected") connected = std::stoi(value);
							else if (field == "capacity") capacity = std::stoi(value);
							else if (field == "port") port = std::stoi(value);
						}

						// 테스트용 출력
						std::cout << "[로비 상태] " << key << " - 접속자: " << connected
							<< " / 최대: " << capacity << " / 포트: " << port << std::endl;

						freeReplyObject(replyData);
					}
				}
			}


		freeReplyObject(reply);
		} while (cursor != 0);
	}
}