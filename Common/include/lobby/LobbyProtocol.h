#pragma once
#include <string>
#include "../utility/MESSAGE_PROTOCOL_generated.h"
#include "../library/flatbuffers/flatbuffers.h"
#include "../lobby/DatabaseProtocol.h"

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

		void NoticeInventory(std::vector<Common::Protocol::InventorySlotData> inventoryItemDataSet, PacketOutput& outPacket);//TODO
		void NoticeInventoryUpdate(std::vector<Common::Protocol::InventorySlotData> updateInventoryDataSet, PacketOutput& outPacket);
		void NoticeInventoryDeleted(std::string& guid);

		void RequestItemBreak(long accountNumber, std::string guid, int& removeCount, PacketOutput& outPacket);
		void ResponseItemBreak(bool feedback, std::string guid, int moneyReward, int removeCount, PacketOutput& outPacket);

		void RequestHeartbeat(PacketOutput& outPacket);
		void ResponseHeartbeat(PacketOutput& outPacket);
	}
}
