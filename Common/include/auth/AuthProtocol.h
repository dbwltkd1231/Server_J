#pragma once
#include <string>
#define NOMINMAX 
#include <winsock2.h>



namespace Common
{
	namespace Auth
	{
		void CreateRequestConnect(const std::string& uid, uint32_t& contentsType, std::string& buffer, int& bodySize);
		void CreateResponseConnect(long accountNumber, std::string uid, bool isNew, const std::string& authToken, int lobbyPort, uint32_t& contentsType, std::string& buffer, int& bodySize);
	}
}
