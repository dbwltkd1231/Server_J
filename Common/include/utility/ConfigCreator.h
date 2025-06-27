#pragma once
#include <iostream>
#include <fstream>
#include <../library/nlohmann/json.hpp> 


namespace Utility
{
	void CreateAuthServerSettingFiles()
	{
		nlohmann::json config;

		/// ��Ʈ��ũ ����
		config["NETWORK"]["IP"] = "127.0.0.1";
		config["NETWORK"]["AUTHPORT"] = 9090;
		config["NETWORK"]["OVERLAPPED_COUNT_MAX"] = 100;
		config["NETWORK"]["BUFFER_SIZE_MAX"] = 1024;
		config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"] = 10;

		config["REDIS"]["PORT"] = 6379;

		// SQL ����
		config["SQL"]["USER_DB_NAME"] = "User";
		config["SQL"]["USER_DB_ADDRESS"] = "DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=User;Trusted_Connection=yes;";



		std::ofstream file("config.json");
		file << config.dump(4);  // 4�� �鿩���� ����

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

	void CreateLobbySettingFiles()
	{
		nlohmann::json config;

		/// ��Ʈ��ũ ����
		config["NETWORK"]["IP"] = "127.0.0.1";
		config["NETWORK"]["Lobby_START_PORT"] = 9091;
		config["NETWORK"]["Lobby_START_COUNT"] = 2;


		config["NETWORK"]["OVERLAPPED_COUNT_MAX"] = 100;
		config["NETWORK"]["BUFFER_SIZE_MAX"] = 1024;
		config["NETWORK"]["CLIENT_ACCEPTREADY_COUNT_MAX"] = 10;
		config["NETWORK"]["CLIENT_CAPACITY"] = 100;

		config["REDIS"]["PORT"] = 6379;

		// SQL ����
		config["SQL"]["USER_DB_NAME"] = "User";
		config["SQL"]["USER_DB_ADDRESS"] = "DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=User;Trusted_Connection=yes;";

		config["SQL"]["GAME_DB_NAME"] = "Game";
		config["SQL"]["GAME_DB_ADDRESS"] = "DRIVER={SQL Server};SERVER=DESKTOP-O5SU309\\SQLEXPRESS;DATABASE=User;Trusted_Connection=yes;";

		std::ofstream file("config.json");
		file << config.dump(4);  // 4�� �鿩���� ����

		std::string log = "[Utility] [JsonCreator] File CreateSuccess !";
		std::cout << log << std::endl;
	}

	nlohmann::json LoadSettingFiles()
	{
		std::ifstream file("config.json");  // JSON ���� ����
		if (!file)
		{
			std::string log = "[Utility] [JsonCreator] File Find Fail !!";
			std::cout << log << std::endl;

			return NULL;
		}

		nlohmann::json config;
		file >> config;  // ���Ͽ��� JSON ��ü �б�

		return config;
	}
}
