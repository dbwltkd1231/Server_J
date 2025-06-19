#pragma once
#include <string>


namespace Database
{
	struct Task
	{
		ULONG_PTR Socket;
		uint32_t MessageType;
		std::string DatabaseName;
		std::string ProcedureName;
		std::string Parameters;
	};

}