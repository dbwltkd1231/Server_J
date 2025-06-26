#pragma once
#include "../auth/NetworkProtocol.h"

namespace Common
{
	namespace Auth
	{
		void CreateRequestConnect(const std::string& uid, uint32_t& contentsType, std::string& buffer, int& bodySize)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto uidOffset = builder.CreateString(uid);
			auto requestConnect = protocol::CreateREQUEST_CONNECT(builder, uidOffset);
			builder.Finish(requestConnect);

			contentsType = static_cast<uint32_t>(protocol::MessageContent_REQUEST_CONNECT);
			bodySize = builder.GetSize();
			buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), bodySize);
		}

		void CreateResponseConnect(std::string uid, bool isNew, const std::string& authToken, int lobbyPort, uint32_t& contentsType, std::string& buffer, int& bodySize)
		{
			flatbuffers::FlatBufferBuilder builder;

			auto authTokenOffset = builder.CreateString(authToken);
			auto uidOffset = builder.CreateString(uid);
			auto responseConnect = protocol::CreateRESPONSE_CONNECT(builder, uidOffset, isNew, authTokenOffset, lobbyPort);

			builder.Finish(responseConnect);

			contentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);
			bodySize = builder.GetSize();
			buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), bodySize);
		}
	}
}