#pragma once
#include <string>
#define NOMINMAX
#include <winsock2.h>

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/game/LOGINSERVER_PROTOCOL_generated.h"


namespace Game
{
	namespace Protocol
	{
		void CreateRequestConnect(const std::string& uid, uint32_t& contentsType, std::string& buffer, int& bodySize);
		void CreateResponseConnect(std::string uid, bool loginSuccess, const std::string& authToken, int lobbyPort, uint32_t& contentsType, std::string& buffer, int& bodySize);
		
	}
}