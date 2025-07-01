#pragma once
#include <string>
#include "../utility/MESSAGE_PROTOCOL_generated.h"
#include "../library/flatbuffers/flatbuffers.h"

namespace Common
{
	namespace Lobby
	{
		struct PacketOutput 
		{
			std::string Buffer;
			int BodySize;
			uint32_t ContentsType;
		};

		void CreateRequestLogIn(long accountNumber, std::string& authToken, PacketOutput& outPacket);
		void CreateResponseLogIn(uint32_t detail, bool feedback, PacketOutput& outPacket);

		void NoticeAccount(std::string& uid, uint64_t& money, int& ranking, int& inventoryCapacity, PacketOutput& outPacket);

		void NoticeInventory();//TODO
		void NoticeInventoryUpdate();
		void NoticeInventoryDeleted(std::string& guid);

		void RequestItemBreak(std::string& guid, int& count, PacketOutput& outPacket);
		void ResponseItemBreak(bool feedback, PacketOutput& outPacket);

		void RequestHeartbeat(PacketOutput& outPacket);
		void ResponseHeartbeat(PacketOutput& outPacket);
	}
}
