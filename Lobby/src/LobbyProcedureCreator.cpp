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
    }
}