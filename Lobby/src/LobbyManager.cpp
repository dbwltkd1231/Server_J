#pragma once
#include <thread>
#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"
#include "LobbyManager.h"
#include "ConstValue.h"
#include "../lobby/LobbyProtocol.h"
#include <LobbyProcedureCreator.h>

#include <openssl/hmac.h>
#include <openssl/evp.h>


namespace Lobby
{
	LobbyManager::LobbyManager()
	{
		_redis = nullptr;
		_serverOn = false;

		_gen.seed(_randomDevice());
	}

	LobbyManager::~LobbyManager()
	{
		_redis = nullptr;
		_serverOn = false;
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

		_callbackProcedureResult = std::function<void(ULONG_PTR, Database::DatabaseQueryType, uint32_t, SQLHSTMT)>
			(
				[this]
				(ULONG_PTR socketPtr, Database::DatabaseQueryType queryType, uint32_t contentsType, SQLHSTMT hstmt)
				{
					this->ProcessQueryResult(socketPtr, queryType, contentsType, hstmt);
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

	//TODO 만료기간도 추가하면 더 좋다.
	bool LobbyManager::verifyJWT(const std::string& token, const std::string& secret)
	{
		size_t pos1 = token.find('.');
		size_t pos2 = token.find('.', pos1 + 1);

		if (pos1 == std::string::npos || pos2 == std::string::npos)
			return false;

		std::string headerPayload = token.substr(0, pos2);
		std::string signature = token.substr(pos2 + 1);

		unsigned char* expectedSig = HMAC(EVP_sha256(),
			secret.data(), secret.size(),
			reinterpret_cast<const unsigned char*>(headerPayload.data()), headerPayload.size(),
			nullptr, nullptr);

		std::string encodedExpectedSig = std::string(reinterpret_cast<char*>(expectedSig), 32);

		return (signature == encodedExpectedSig); 
	}

	void LobbyManager::ProcessAccept(ULONG_PTR& targetSocket)
	{
		_notLoginSocketSet.insert(targetSocket);

		Lobby::EventWorker loginEvent;
		loginEvent.Initialize(targetSocket, std::chrono::steady_clock::now(), std::chrono::seconds(30), Lobby::EventType::Login);

		_eventQueue.push(std::move(loginEvent));
	}

	//find부분.. 함수로 따로만들면 좋을듯.
	void LobbyManager::ProcessDisconnect(ULONG_PTR& targetSocket, int errorCode)
	{
		auto finder2 = _notLoginSocketSet.find(targetSocket);
		if (finder2 != _notLoginSocketSet.end())
		{
			_notLoginSocketSet.unsafe_erase(targetSocket);
		}

		auto finder = _socketLoginAccountMap.find(targetSocket);

		//LogOut처리.
		if (finder != _socketLoginAccountMap.end())
		{
			auto accountNumber = finder->second;
			auto userLogOutTask = Common::Lobby::CreateQuerryUserLogOut(targetSocket, accountNumber);
			_socketLoginAccountMap.unsafe_erase(targetSocket);

			_userDatabaseWorker.Enqueue(std::move(userLogOutTask));
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

				bool tokenBerify = verifyJWT(authToken, "YJS");

				if (tokenBerify)
				{
					Utility::Log("Lobby", "LobbyManager", " 인증토큰 검증 완료.");
					task = Common::Lobby::CreateQuerryUserLogIn(targetSocket, accountNumber);
				}
				
				break;
			}

			case protocol::MessageContent_REQUEST_ITEM_BREAK:
			{
				auto requestItemBreak = flatbuffers::GetRoot<protocol::REQUEST_ITEM_BREAK>(buffer);
				long accountNumber = requestItemBreak->account_number();
				std::string guid = requestItemBreak->guid()->str();
				int removeCount = requestItemBreak->remove_count();

				task = Common::Lobby::CreateQuerryBreakInventoryItem(targetSocket, accountNumber, guid, removeCount);
			}
		}

		if (task.DatabaseName == Database::DatabaseType::User)
		{
			_userDatabaseWorker.Enqueue(std::move(task));
		}
		else if (task.DatabaseName == Database::DatabaseType::Game)
		{
			_gameDatabaseWorker.Enqueue(std::move(task));
		}
		else
		{

		}
	}

	void LobbyManager::ProcessQueryResult(ULONG_PTR targetSocket, Database::DatabaseQueryType queryType, uint32_t contentsType, SQLHSTMT& hstmt)
	{
		switch (queryType)
		{
		case Database::DatabaseQueryType::GetItemAllData:
		{
			SetProcedureResult(_gameItemVector, hstmt);
			Utility::Log("Lobby", "LobbyManager", "아이템 데이터 로드 : " + std::to_string(_gameItemVector.size()));
			break;
		}
		case Database::DatabaseQueryType::UserLogIn:
		{
			Common::Lobby::PacketOutput output;
			Common::Protocol::ResultUserLogIn userLoginResult;
			SetProcedureResult(userLoginResult, hstmt);

			Common::Lobby::CreateResponseLogIn(userLoginResult.Detail, userLoginResult.Success, output);
			_networkManager.SendRequest(targetSocket, contentsType, output.Buffer, output.BodySize);

			//로비 상태 저장

			if (userLoginResult.Success)
			{
				_socketLoginAccountMap.insert({ targetSocket, userLoginResult.AccountNumber });
				Utility::Log("Lobby", "LobbyManager", "현재 로그인 성공 인원 : " + std::to_string(_socketLoginAccountMap.size()));

				auto finder = _notLoginSocketSet.find(targetSocket);
				if (finder != _notLoginSocketSet.end())
				{
					_notLoginSocketSet.unsafe_erase(targetSocket);
				}

				auto accountDataTask = Common::Lobby::CreateQuerryAccountData(targetSocket, userLoginResult.AccountNumber);
				_userDatabaseWorker.Enqueue(std::move(accountDataTask));

				auto inventoryByAccountDataTask = Common::Lobby::CreateQuerryInventoryByAccount(targetSocket, userLoginResult.AccountNumber);
				_gameDatabaseWorker.Enqueue(std::move(inventoryByAccountDataTask));

				Lobby::EventWorker itemGiveEvent;
				itemGiveEvent.Initialize(targetSocket, std::chrono::steady_clock::now(), std::chrono::seconds(15), Lobby::EventType::ItemGive);

				_eventQueue.push(std::move(itemGiveEvent));
			}
			else if (!userLoginResult.Success)
			{
				Utility::Log("Lobby", "LobbyManager", "로그인 실패 ?");
			}
			break;
		}
		case Database::DatabaseQueryType::GetAccountData:
		{
			Common::Lobby::PacketOutput output;
			Common::Protocol::ResultGetAccountData getAccountDataResult;
			SetProcedureResult(getAccountDataResult, hstmt);

			Common::Lobby::NoticeAccount(getAccountDataResult.AccountUID, getAccountDataResult.GameMoney, getAccountDataResult.GameMoneyRank, getAccountDataResult.InventoryCapacity, output);
			_networkManager.SendRequest(targetSocket, contentsType, output.Buffer, output.BodySize);
			break;
		}
		case Database::DatabaseQueryType::GetInventoryByAccount:
		{
			Common::Lobby::PacketOutput output;
			Common::Protocol::ResultGetInventoryData getInventoryDataResult;
			SetProcedureResult(getInventoryDataResult, hstmt);

			Common::Lobby::NoticeInventory(getInventoryDataResult.InventoryItems, output);
			_networkManager.SendRequest(targetSocket, contentsType, output.Buffer, output.BodySize);
			break;
		}
		case Database::DatabaseQueryType::AddInventoryItem:
		{
			Common::Lobby::PacketOutput output;
			Common::Protocol::ResultAddInventoryItem addInventoryItemResult;
			SetProcedureResult(addInventoryItemResult, hstmt);
			
			Common::Lobby::NoticeInventoryUpdate(addInventoryItemResult.UpdateInventoryDataSet, output);
			_networkManager.SendRequest(targetSocket, contentsType, output.Buffer, output.BodySize);

			Lobby::EventWorker itemGiveEvent;
			itemGiveEvent.Initialize(targetSocket, std::chrono::steady_clock::now(), std::chrono::seconds(15), Lobby::EventType::ItemGive);

			_eventQueue.push(std::move(itemGiveEvent));
			break;
		}
		case Database::DatabaseQueryType::BreakInventoryItem:
		{
			Common::Lobby::PacketOutput output;
			Common::Protocol::ResultBreakInventoryItem resultBreakInventoryItem;
			SetProcedureResult(resultBreakInventoryItem, hstmt);
			bool success = (resultBreakInventoryItem.Success == 1 ? true : false);
			Common::Lobby::ResponseItemBreak(success, resultBreakInventoryItem.GuidStr, resultBreakInventoryItem.MoneyReward, resultBreakInventoryItem.RemoveCount, output);
			_networkManager.SendRequest(targetSocket, contentsType, output.Buffer, output.BodySize);
			break;
		}
		default:
		{
			break;
		}
		}
	}

	// promise나 condition varaiable등 쓰면 더 좋을듯...
	void LobbyManager::EventThread()
	{
		Lobby::EventWorker eventWorker;
		while (_serverOn)
		{
			if (!_eventQueue.try_pop(eventWorker))
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			if (eventWorker.TimerCheck())
			{
				EventProcess(eventWorker.TargetSocket, eventWorker.Event);
				continue;
			}

			_eventQueue.push(std::move(eventWorker));
		}
	}
	
	void LobbyManager::EventProcess(ULONG_PTR& targetSocket, Lobby::EventType eventType)
	{
		switch (eventType)
		{
			case Lobby::EventType::Default:
				break;
			case Lobby::EventType::Login:
			{
				//접속 후 60초 이내 인증 처리되지 않은 클라이언트 접속 끊는 기능.
				auto finder = _socketLoginAccountMap.find(targetSocket);
				if (finder != _socketLoginAccountMap.end())
				{
					Utility::Log("Lobby", "LobbyManager", "60초 이내 접속 성공확인"+std::to_string(targetSocket));
					break;
				}

				_networkManager.DisconnectRequest(targetSocket);
				Utility::Log("Lobby", "LobbyManager", "60초 이내 접속 실패 -> 연결해제 시도.." + std::to_string(targetSocket));
				break;
			}
			case Lobby::EventType::HeartBeat:
			{
				break;
			}
			case Lobby::EventType::ItemGive:
			{
				auto finder = _socketLoginAccountMap.find(targetSocket);
				if (finder == _socketLoginAccountMap.end())
				{
					Utility::Log("Lobby", "LobbyManager", "접속중이지 않아 ItemGive 이벤트 취소.");
					break;
				}
				std::uniform_int_distribution<int> giveCount(1, 3);
				int itemGiveCount = giveCount(_gen);
				
				std::uniform_int_distribution<int> randomItem(0, _gameItemVector.size() - 1);
				int randomItemIndex = randomItem(_gen);
				int randomItemSeed = _gameItemVector[randomItemIndex].ItemSeed;
				auto addInventoryItemTask = Common::Lobby::CreateQuerryAddInventoryItem(targetSocket, finder->second, randomItemSeed, itemGiveCount);
				_gameDatabaseWorker.Enqueue(std::move(addInventoryItemTask));
				break;
			}
			case Lobby::EventType::End:
				break;
		}
	}
	
	// main에서 JOIN으로 호출하자
	void LobbyManager::MainProcess()
	{
		auto task = Common::Lobby::CreateQueryGetItemData();
		_gameDatabaseWorker.Enqueue(task);

		_serverOn = true;

		std::thread eventThread([this]() { this->EventThread(); });
		eventThread.detach();

		while (_serverOn)
		{
			//접속 후 60초 이내 인증 처리되지 않은 클라이언트 접속 끊는 기능 추가

			//유저는 로비 서버 접속 후 60초마다 아이템을 1~3개 랜덤으로 지급받는다. -> 둘다처리할수있는 타이머객체가 하나있으면 좋으려나? 만들수있을까?
		}
	}
}
