#pragma once
#include <string>


namespace Database
{
	enum DatabaseType
	{
		Default = -1,
		User,
		Game
	};

	enum DatabaseQueryType
	{
		//User
	
		CheckAndAddAccount,
		DeleteAccount,
		UserLogIn,
		UserLogOut,
		GetAccountData,
		UpdateUserMoney,


		//Game
		GetItemAllData,
		GetInventoryByAccount,
		AddInventoryItem,
		DeleteInventoryItem,
		BreakInventoryItem

	};

	struct Task
	{
		ULONG_PTR SocketPtr;
		DatabaseQueryType QueryType;
		uint32_t NetworkType;
		DatabaseType DatabaseName = DatabaseType::Default;
		std::string ProcedureName;
		std::string Parameters;
	};

}