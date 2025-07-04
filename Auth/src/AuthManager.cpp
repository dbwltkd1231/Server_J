#pragma once

#include "ConstValue.h"
#include "AuthManager.h"
#include "../auth/AuthProtocol.h"
#include "../auth/DatabaseProtocol.h"
#include "../auth/BasicData.h"
#include "../utility/Debug.h"


namespace Auth
{
	AuthManager::AuthManager()
	{
		isOn = false;
	}

	AuthManager::~AuthManager()
	{
		isOn = false;
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

		_databaseCallback = std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT)>
			(
				[this]
				(ULONG_PTR socketPtr, Database::DatabaseQueryType queryType, uint32_t contentsType, SQLHSTMT hstmt)
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

	void AuthManager::Process()
	{
		isOn = true;

		while (isOn)
		{

		}
	}

	void AuthManager::ProcessAccept(ULONG_PTR& targetSocket)
	{
		Utility::Log("Auth", "AuthManager", "ProcessAccept");
	}

	void AuthManager::ProcessDisconnect(ULONG_PTR& targetSocket, int errorCode)
	{
		Utility::Log("Auth", "AuthManager", "ProcessDisconnect");
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

				task = Common::Auth::CreateQuerryAccountCheck(targetSocket, loginId);
				break;
			}
		}

		if (task.DatabaseName == Database::DatabaseType::User)
		{
			_userDatabaseWorker.Enqueue(std::move(task));
		}
		else if (task.DatabaseName == Database::DatabaseType::Game)
		{
			//Auth서버는 user데이터베이스만 사용중..
		}
		else
		{

		}
	}

	void AuthManager::DatabaseCallback(ULONG_PTR targetSocket, uint32_t contentsType, SQLHSTMT& hstmt)
	{
		std::shared_ptr<Common::Auth::BasicData> result = Common::Auth::GetSqlData(targetSocket, contentsType, hstmt);

		auto contentsTypeOffset = static_cast<protocol::MessageContent> (result->ContentsType);
		std::string stringBuffer;
		int bodySize = 0;
		switch (contentsType)
		{
			case protocol::MessageContent_REQUEST_CONNECT:
			{
				auto requestConnectData = std::static_pointer_cast<Common::Auth::RequestConnectData>(result);

				std::string accountNumberStr = std::to_string(requestConnectData->AccountNumber);
				auto token = createJWT(accountNumberStr, Auth::ConstValue::GetInstance().SecretKey);
				int targetPort = CheckLobbyServerState();
				if (targetPort == -1)
				{
					Utility::Log("Auth", "AuthManager", "유효한 로비서버 포트번호 조회 실패");
				}
				else
				{
					Common::Auth::CreateResponseConnect(requestConnectData->AccountNumber, requestConnectData->AccountUID, requestConnectData->IsNew, token, targetPort, contentsType, stringBuffer, bodySize);
				}
				break;
			}
			default:
				break;
		}

		_networkManager.SendRequest(targetSocket, contentsType, stringBuffer, bodySize);
	}

	int AuthManager::CheckLobbyServerState()
	{
		int currentTargetLobbyPort = -1;
		int targetLobbyServerRemain = 0;

		unsigned long long cursor = 0;
		do {
			redisReply* reply = (redisReply*)redisCommand(_redis, "SCAN %llu MATCH Lobby:* COUNT 10", cursor);

			if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) // key, element구조이기때문에 최소2개가있어야 데이터1개를 조회할수있다.
			{
				continue;
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

						if (connected && capacity > targetLobbyServerRemain)
						{
							currentTargetLobbyPort = port;
							targetLobbyServerRemain = capacity;
						}

						std::cout << "[로비 이름] " << key << " - 상태: " << connected
							<< " / 수용가능량: " << capacity << " / 포트: " << port << std::endl;

						freeReplyObject(replyData);
					}
				}
			}

		freeReplyObject(reply);
		} while (cursor != 0);

		return currentTargetLobbyPort;
	}

	std::string AuthManager::createJWT(const std::string& userId, const std::string& secret)
	{
		std::string header = R"({"alg":"HS256","typ":"JWT"})";
		std::string payload = R"({"sub":")" + userId + R"(","exp":1727816400})"; // 예: Unix timestamp

		std::string encodedHeader = header;//Base64Encode(header);
		std::string encodedPayload = payload;//Base64Encode(payload);

		std::string dataToSign = encodedHeader + "." + encodedPayload;

		// HMAC-SHA256 서명
		unsigned char* signature = HMAC(EVP_sha256(),
			secret.data(), secret.size(),
			reinterpret_cast<const unsigned char*>(dataToSign.data()), dataToSign.size(),
			nullptr, nullptr);

		//std::string encodedSignature = Base64Encode(std::string(reinterpret_cast<char*>(signature), 32));
		std::string encodedSignature = std::string(reinterpret_cast<char*>(signature), 32);
		return dataToSign + "." + encodedSignature;
	}


}