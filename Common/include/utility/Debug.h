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

		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4); ->  �������ǹ߻�

		//std::cout << "\x1b[1;31m"  // ���� ������
		//	<< "[ERROR] " << message
		//	<< "\x1b[0m"     // ���� �ʱ�ȭ
		//	<< std::endl;  -> �������ǹ߻�

		//lock�� ����ŭ �ʿ��� �κ��� �ƴ϶�� �����ؼ�  �Ʒ��� ���� ����.

		std::string message = "[ Error ] [" + space + "] [" + script + "] " + log + "\n";
		std::cout << message;

	}
}