#pragma once
#include "LobbyProcedureCreator.h"

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"

namespace Common
{
    namespace Lobby
    {
        Database::Task CreateQueryGetItemData()
        {
            Database::Task task;
            task.NetworkType = -1;
            task.QueryType = Database::DatabaseQueryType::GetItemAllData;
            task.DatabaseName = Database::DatabaseType::Game;
            task.ProcedureName = "GetItemAllData";
            task.Parameters = "";
            return task;
        }

        Database::Task CreateQuerryUserLogIn(ULONG_PTR targetSocket, long accountNumber)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.NetworkType = protocol::MessageContent_RESPONSE_LOGIN;
            task.QueryType = Database::DatabaseQueryType::UserLogIn;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "UserLogIn";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }

        Database::Task CreateQuerryAccountData(ULONG_PTR targetSocket, long accountNumber)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.NetworkType = protocol::MessageContent_NOTICE_ACCOUNT;
            task.QueryType = Database::DatabaseQueryType::GetAccountData;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "GetAccountData";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }

        Database::Task CreateQuerryUserLogOut(ULONG_PTR targetSocket, long accountNumber)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.NetworkType = -1;
            task.QueryType = Database::DatabaseQueryType::UserLogOut;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "UserLogOut";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }

        Database::Task CreateQuerryInventoryByAccount(ULONG_PTR targetSocket, long accountNumber)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.NetworkType = protocol::MessageContent_NOTICE_INVENTORY;
            task.QueryType = Database::DatabaseQueryType::GetInventoryByAccount;
            task.DatabaseName = Database::DatabaseType::Game;
            task.ProcedureName = "GetInventoryByAccount";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }
    }
}