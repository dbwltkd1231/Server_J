#pragma once
#define NOMINMAX 
#include "WinSock2.h"

#include "../database/Task.h"

namespace Common
{
    namespace Lobby
    {
        Database::Task CreateQuerryUserLogIn(ULONG_PTR targetSocket, long accountNumber, uint32_t contentsType);
        Database::Task CreateQuerryAccountData(ULONG_PTR targetSocket, long accountNumber, uint32_t contentsType);
    }
}