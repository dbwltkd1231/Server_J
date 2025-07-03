#pragma once
#include "LobbyProcedureCreator.h"

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"

namespace Common
{
    namespace Lobby
    {
        Database::Task CreateQuerryUserLogIn(ULONG_PTR targetSocket, long accountNumber, uint32_t contentsType)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.MessageType = contentsType;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "UserLogIn";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }

        Database::Task CreateQuerryAccountData(ULONG_PTR targetSocket, long accountNumber, uint32_t contentsType)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.MessageType = contentsType;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "GetAccountData";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }

        Database::Task CreateQuerryUserLogOut(ULONG_PTR targetSocket, long accountNumber, uint32_t contentsType)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.MessageType = contentsType;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "UserLogOut";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }

        Database::Task CreateQuerryInventoryByAccount(ULONG_PTR targetSocket, long accountNumber, uint32_t contentsType)
        {
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.MessageType = contentsType;
            task.DatabaseName = Database::DatabaseType::Game;
            task.ProcedureName = "GetInventoryByAccount";
            task.Parameters = " '" + std::to_string(accountNumber) + "'";
            return task;
        }
    }
}