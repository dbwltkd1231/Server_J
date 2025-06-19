#pragma once
#include <string>


namespace Database
{
	struct Task
	{
		ULONG_PTR Socket;
		int MessageType;
		std::string DatabaseName;
		std::string ProcedureName;
		std::string Parameters;
	};

}