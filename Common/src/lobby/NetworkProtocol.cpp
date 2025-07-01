
#include "../lobby/NetworkProtocol.h"

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

		void NoticeAccount(int64_t& accountNumber, std::string& uid, int64_t& money, int& ranking, int& inventoryCapacity, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto uidOffset = builder.CreateString(uid);
			auto noticeAccount = protocol::CreateNOTICE_ACCOUNT(builder, accountNumber, uidOffset, money, ranking, inventoryCapacity);
			builder.Finish(noticeAccount);

			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);

		}

		void NoticeInventory()
		{

		}
		void NoticeInventoryUpdate()
		{

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