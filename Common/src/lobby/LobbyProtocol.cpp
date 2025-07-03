
#include "../lobby/LobbyProtocol.h"

namespace Common
{
	namespace Lobby
	{
		void CreateRequestLogIn(long accountNumber, std::string& authToken, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto tokenOffset = builder.CreateString(authToken);

			auto requestConnect = protocol::CreateREQUEST_LOGIN(builder, accountNumber, tokenOffset);
			builder.Finish(requestConnect);

			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
			outPacket.ContentsType = protocol::MessageContent_REQUEST_LOGIN;

		}
		void CreateResponseLogIn(uint32_t detail, bool feedback, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			protocol::FEEDBACK_LOGIN detailOffset = static_cast<protocol::FEEDBACK_LOGIN>(detail);
			auto responseConnect = protocol::CreateRESPONSE_LOGIN(builder, detailOffset, feedback);
			builder.Finish(responseConnect);

			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
		}

		void NoticeAccount(std::string& uid, uint64_t& money, int& ranking, int& inventoryCapacity, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto uidOffset = builder.CreateString(uid);
			auto noticeAccount = protocol::CreateNOTICE_ACCOUNT(builder, uidOffset, money, ranking, inventoryCapacity);
			builder.Finish(noticeAccount);

			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);

		}

		void NoticeInventory(std::vector<Common::Protocol::InventorySlotData> inventoryItemDataSet, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			std::vector<flatbuffers::Offset<protocol::INVENTORY_SLOT>> slotOffsets;

			for (const auto& item : inventoryItemDataSet)
			{
				auto guidOffset = builder.CreateString(item.GuidStr);
				auto slot = protocol::CreateINVENTORY_SLOT(builder, guidOffset, item.ItemSeed, item.ItemCount);
				slotOffsets.push_back(slot);
			}

			auto slotVector = builder.CreateVector(slotOffsets);
			auto root = protocol::CreateNOTICE_INVENTORY(builder, slotVector, inventoryItemDataSet.size());
			builder.Finish(root);

			outPacket.BodySize = static_cast<int>(builder.GetSize());
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
		}

		void NoticeInventoryUpdate(std::vector<Common::Protocol::InventorySlotData> updateInventoryDataSet, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			std::vector<flatbuffers::Offset<protocol::INVENTORY_SLOT>> slotOffsets;

			for (const auto& item : updateInventoryDataSet)
			{
				auto guidOffset = builder.CreateString(item.GuidStr);
				auto slot = protocol::CreateINVENTORY_SLOT(builder, guidOffset, item.ItemSeed, item.ItemCount);
				slotOffsets.push_back(slot);
			}

			auto slotVector = builder.CreateVector(slotOffsets);
			auto root = protocol::CreateNOTICE_INVENTORY_UPDATE(builder, slotVector, updateInventoryDataSet.size());
			builder.Finish(root);

			outPacket.BodySize = static_cast<int>(builder.GetSize());
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
		}

		void NoticeInventoryDeleted(std::string& guid)
		{

		}

		void RequestItemBreak(std::string& guid, int& count, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto guidOffset = builder.CreateString(guid);
			auto requestItemBreak = protocol::CreateREQUEST_ITEM_BREAK(builder, guidOffset, count);
			builder.Finish(requestItemBreak);

			outPacket.ContentsType = static_cast<uint32_t>(protocol::MessageContent_REQUEST_ITEM_BREAK);
			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
		}
		void ResponseItemBreak(bool feedback, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto responseItemBreak = protocol::CreateRESPONSE_ITEM_BREAK(builder, feedback);
			builder.Finish(responseItemBreak);

			outPacket.ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_ITEM_BREAK);
			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
		}

		void RequestHeartbeat(PacketOutput& outPacket)
		{

		}
		void ResponseHeartbeat(PacketOutput& outPacket)
		{

		}

	}
}