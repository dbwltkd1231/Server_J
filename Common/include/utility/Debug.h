#pragma once
#include <iostream>
#include <string>

#include <windows.h>

namespace Utility
{
	static void Log(const std::string space, const std::string script, const std::string log)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

		std::string result = "[" + space + "] [" + script + "] " + log + "\n";
		std::cout << result;
	}

	static void LogError(const std::string space, const std::string script, const std::string log)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);

		std::string result = "[" + space + "] [" + script + "] " + log + "\n";
		std::cout << result;
	}
}