#pragma once
#include <iostream>
#include <string>


namespace Utility
{
	static void Log(const std::string space, const std::string script, const std::string log)
	{
		std::string message = "[" + space + "] [" + script + "] " + log + "\n";
		std::cout << message;
	}

	static void LogError(const std::string space, const std::string script, const std::string log)
	{
		//std::string message = "[" + space + "] [" + script + "] " + log + "\n";

		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4); ->  경쟁조건발생

		//std::cout << "\x1b[1;31m"  // 굵은 빨간색
		//	<< "[ERROR] " << message
		//	<< "\x1b[0m"     // 색상 초기화
		//	<< std::endl;  -> 경쟁조건발생

		//lock을 쓸만큼 필요한 부분은 아니라고 생각해서  아래와 같이 수정.

		std::string message = "[ Error ] [" + space + "] [" + script + "] " + log + "\n";
		std::cout << message;

	}
}