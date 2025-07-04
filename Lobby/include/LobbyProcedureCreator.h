#pragma once
#define NOMINMAX 
#include "WinSock2.h"

#include "../database/Task.h"

namespace Common
{
    namespace Lobby
    {
        Database::Task CreateQueryGetItemData();
        Database::Task CreateQuerryUserLogIn(ULONG_PTR targetSocket, long accountNumber);
        Database::Task CreateQuerryAccountData(ULONG_PTR targetSocket, long accountNumber);
        Database::Task CreateQuerryUserLogOut(ULONG_PTR targetSocket, long accountNumber);
        Database::Task CreateQuerryInventoryByAccount(ULONG_PTR targetSocket, long accountNumber);
        Database::Task CreateQuerryAddInventoryItem(ULONG_PTR targetSocket, long accountNumber, long itemSeed, int itemCount);
        Database::Task CreateQuerryBreakInventoryItem(ULONG_PTR targetSocket, long accountNumber, std::string guid, int removeCount);
    }
}