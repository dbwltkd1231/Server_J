#pragma once
#include "../lobby/NetworkProtocol.h"

namespace Common
{
	namespace Lobby
	{
		void CreateRequestConnect(std::string& uid, std::string& authToken, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto uidOffset = builder.CreateString(uid);
			auto tokenOffset = builder.CreateString(authToken);

			auto requestConnect = protocol::CreateREQUEST_CONNECT(builder, uidOffset, tokenOffset);
			builder.Finish(requestConnect);

			outPacket.ContentsType = static_cast<uint32_t>(protocol::MessageContent_REQUEST_CONNECT);
			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);

		}
		void CreateResponseConnect(bool feedback, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto responseConnect = protocol::CreateRESPONSE_CONNECT(builder, feedback);
			builder.Finish(responseConnect);

			outPacket.ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);
			outPacket.BodySize = builder.GetSize();
			outPacket.Buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), outPacket.BodySize);
		}

		void NoticeAccount(int64_t& accountNumber, std::string& uid, int64_t& money, int& ranking, int& inventoryCapacity, PacketOutput& outPacket)
		{
			flatbuffers::FlatBufferBuilder builder;
			auto uidOffset = builder.CreateString(uid);
			auto noticeAccount = protocol::CreateNOTICE_ACCOUNT(builder, accountNumber, uidOffset, money, ranking, inventoryCapacity);
			builder.Finish(noticeAccount);

			outPacket.ContentsType = static_cast<uint32_t>(protocol::MessageContent_NOTICE_ACCOUNT);
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


	//void CreateRequestConnect(const std::string& uid, uint32_t& contentsType, std::string& buffer, int& bodySize)
	//{
	//	flatbuffers::FlatBufferBuilder builder;
	//	auto uidOffset = builder.CreateString(uid);
	//	auto requestConnect = protocol::CreateREQUEST_CONNECT(builder, uidOffset);
	//	builder.Finish(requestConnect);
	//
	//	contentsType = static_cast<uint32_t>(protocol::MessageContent_REQUEST_CONNECT);
	//	bodySize = builder.GetSize();
	//	buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), bodySize);
	//}
	//
	//void CreateResponseConnect(std::string uid, bool isNew, const std::string& authToken, int lobbyPort, uint32_t& contentsType, std::string& buffer, int& bodySize)
	//{
	//	flatbuffers::FlatBufferBuilder builder;
	//
	//	auto authTokenOffset = builder.CreateString(authToken);
	//	auto uidOffset = builder.CreateString(uid);
	//	auto responseConnect = protocol::CreateRESPONSE_CONNECT(builder, uidOffset, isNew, authTokenOffset, lobbyPort);
	//
	//	builder.Finish(responseConnect);
	//
	//	contentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);
	//	bodySize = builder.GetSize();
	//	buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), bodySize);
	//}
	}
}