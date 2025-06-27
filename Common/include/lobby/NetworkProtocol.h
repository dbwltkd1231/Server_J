#pragma once
#include <string>
#define NOMINMAX
#include <winsock2.h>

#include "../library/flatbuffers/flatbuffers.h"
#include "LOBBYSERVER_PROTOCOL_generated.h"

namespace Common
{
	namespace Lobby
	{
		struct PacketOutput 
		{
			uint32_t ContentsType;
			std::string Buffer;
			int BodySize;
		};

		void CreateRequestConnect(std::string& uid, std::string& authToken, PacketOutput& outPacket);
		void CreateResponseConnect(bool feedback, PacketOutput& outPacket);

		void NoticeAccount(int64_t& accountNumber, std::string& uid, int64_t& money, int& ranking, int& inventoryCapacity, PacketOutput& outPacket);

		void NoticeInventory();//TODO
		void NoticeInventoryUpdate();
		void NoticeInventoryDeleted(std::string& guid);

		void RequestItemBreak(std::string& guid, int& count, PacketOutput& outPacket);
		void ResponseItemBreak(bool feedback, PacketOutput& outPacket);

		void RequestHeartbeat(PacketOutput& outPacket);
		void ResponseHeartbeat(PacketOutput& outPacket);
	}
}
