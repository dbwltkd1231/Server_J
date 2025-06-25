#pragma once
#include <iostream>
#include <fstream>
#include <../library/nlohmann/json.hpp> 
//#include "../utility/Debug.h"

namespace Utility
{
	void CreateAuthServerSettingFiles()
	{
		nlohmann::json config;

		/// 네트워크 설정
		config["NETWORK"]["IP"] = "127.0.0.1";
		config["NETWORK"]["AUTHPORT"] = 9090;
		config["NETWORK"]["OVERLAPPED_COUNT_MAX"] = 100;
		config["NETWORK"]["BUFFER_SIZE_MAX"] = 1024;
		config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"] = 5;
		config["NETWORK"]["CLIENT_ACTIVATE_COUNT_MAX"] = 10;
		config["NETWORK"]["SOCKET_ONETIME_PREPARE_COUNT"] = 3;

		config["REDIS"]["PORT"] = 6379;

		// SQL 설정
		config["SQL"]["USER_DB_NAME"] = "User";
		config["SQL"]["USER_DB_ADDRESS"] = "DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=User;Trusted_Connection=yes;";



		std::ofstream file("config.json");
		file << config.dump(4);  // 4는 들여쓰기 수준

		std::string log = "[Utility] [JsonCreator] File CreateSuccess !";
		std::cout << log << std::endl;
	}


	void CreateClientSettingFiles()
	{
		nlohmann::json config;

		config["NETWORK"]["IP"] = "127.0.0.1";
		config["NETWORK"]["AUTHPORT"] = 9090;
		config["NETWORK"]["BUFFER_SIZE_MAX"] = 1024;
		config["NETWORK"]["OVERLAPPED_COUNT_MAX"] = 100;

		config["CLIENT"]["CLIENT_TEST_UID"] = "TESTER";
		config["CLIENT"]["TEST_CLIENT_COUNT"] = 10;
		config["CLIENT"]["TEST_THREAD_COUNT"] = 5;

		std::ofstream file("config.json");
		file << config.dump(4);

		std::string log = "[Utility] [JsonCreator] File CreateSuccess !";
		std::cout << log << std::endl;
	}

	nlohmann::json LoadSettingFiles()
	{
		std::ifstream file("config.json");  // JSON 파일 열기
		if (!file)
		{
			std::string log = "[Utility] [JsonCreator] File Find Fail !!";
			std::cout << log << std::endl;

			return NULL;
		}

		nlohmann::json config;
		file >> config;  // 파일에서 JSON 객체 읽기

		return config;
	}
}
