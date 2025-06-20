#pragma once


#include "../include/utility/ConstValue.h"

#include "../auth/AuthManager.h"
#include "../auth/NetworkProtocol.h"
#include "../auth/DatabaseProtocol.h"
#include "../auth/BasicData.h"


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
		_networkManager.Construct();
		for (int i = 0;i < Utility::ConstValue::GetInstance().ConnectReadyClientCountMax;++i)
		{
			_networkManager.ActivateClient();
		}
		std::string clientLog = "Client : " + std::to_string(Utility::ConstValue::GetInstance().ConnectReadyClientCountMax) + " Activate Success !!";
		Utility::Log("Auth", "AuthManager", clientLog);

		_networkManager.ReadMessage = std::function<void(ULONG_PTR&, uint32_t, std::string)>
			(
				[this]
				(ULONG_PTR& targetSocket, uint32_t contentsType, std::string buffer)
				{
					this->ReadMessage(targetSocket, contentsType, buffer);
				}
			);
	}

	void AuthManager::ConnectDatabase(std::string databaseName, std::string sqlServerAddress)
	{
		_userDatabaseWorker.Initialize(databaseName, sqlServerAddress);
		_userDatabaseWorker.Activate(true);
		Utility::Log("Auth", "AuthManager", "UserDatabase Worker Process..");
	}

	void AuthManager::ReadMessage(ULONG_PTR& targetSocket, uint32_t contentsType, std::string stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();

		switch (messageType)
		{
			case protocol::MessageContent_REQUEST_CONNECT:
			{
				auto requestConnect = flatbuffers::GetRoot<protocol::REQUEST_CONNECT>(buffer);

				std::string loginId = requestConnect->login_id()->str();
				
				auto task = Auth::CreateRequestConnect(targetSocket, loginId);
				_userDatabaseWorker.Enqueue(std::move(task));

				break;
			}
		}
	}

	//TODO 콜백연결안되어있음.
	void AuthManager::DatabaseCallback(ULONG_PTR& targetSocket, uint32_t& contentsType, SQLHSTMT& hstmt)
	{
		std::shared_ptr<Auth::BasicData> result = Auth::ReadReturnData(targetSocket, contentsType, hstmt);

		auto contentsTypeOffset = static_cast<protocol::MessageContent> (result->ContentsType);
		std::string stringBuffer;
		int bodySize = 0;
		switch (contentsType)
		{
			case protocol::MessageContent_RESPONSE_CONNECT:
			{
				auto requestConnectData = std::static_pointer_cast<Auth::RequestConnectData>(result);
				CreateResponseConnect(requestConnectData->IsSuccess, "TOKEN", Utility::ConstValue::GetInstance().ServerPort, contentsType, stringBuffer, bodySize);
				break;
			}
			default:
				break;
		}

		_networkManager.SendRequest(targetSocket, contentsType, stringBuffer, bodySize);
		//success -> 토큰발급
	}
}