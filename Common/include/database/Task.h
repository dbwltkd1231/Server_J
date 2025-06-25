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

	struct Task
	{
		ULONG_PTR SocketPtr;
		uint32_t MessageType;
		DatabaseType DatabaseName = DatabaseType::Default;
		std::string ProcedureName;
		std::string Parameters;
	};

}