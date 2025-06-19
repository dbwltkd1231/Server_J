#pragma once

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/database/LOGINSERVER_PROTOCOL_generated.h"
#include "../include/utility/ConstValue.h"

#include "../auth/AuthManager.h"

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
		networkManager.Construct();
		for (int i = 0;i < Utility::CLIENT_ACCEPTREADY_COUNT_MAX;++i)
		{
			networkManager.ActivateClient();
		}
		std::string clientLog = "Client : " + std::to_string(Utility::CLIENT_ACCEPTREADY_COUNT_MAX) + " Activate Success !!";
		Utility::Log("Auth", "AuthManager", clientLog);

		networkManager.ReadMessage = std::function<void(ULONG_PTR, uint32_t, std::string)>
			(
				[this]
				(ULONG_PTR targetSocket, uint32_t contentsType, std::string buffer)
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

	void AuthManager::ReadMessage(ULONG_PTR targetSocket, uint32_t contentsType, std::string stringValue)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);
		const char* buffer = stringValue.c_str();

		switch (messageType)
		{
			case protocol::MessageContent_REQUEST_CONNECT:
			{
				auto requestConnect = flatbuffers::GetRoot<protocol::REQUEST_CONNECT>(buffer);

				std::string loginId = requestConnect->login_id()->str();
				std::string loginPassword = requestConnect->login_password()->str();

				break;
			}
			case protocol::MessageContent_RESPONSE_CONNECT:
			{
				flatbuffers::FlatBufferBuilder builder;
				builder.Finish(protocol::CreateRESPONSE_CONNECT(builder));
				break;
			}
		}
	}

    void AuthManager::LoginRequest(ULONG_PTR targetSocket, std::string loginId)  
    {  
       Database::Task task;  
       task.DatabaseName = "User";  
	   task.MessageType = protocol::MessageContent_REQUEST_CONNECT;
       task.ProcedureName = "CheckAndAddAccount";  
       task.Parameters = loginId;  

       _userDatabaseWorker.Enqueue(std::move(task));  
    }
}