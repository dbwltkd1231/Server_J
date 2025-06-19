#pragma once

#include <winsock2.h>

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/database/LOGINSERVER_PROTOCOL_generated.h"

#include "../database/Task.h"

namespace Auth
{
	Database::Task CreateRequestConnect(ULONG_PTR targetSocket, std::string userId)
	{
		// �α��� ��û ó�� ����
		// ��: �����ͺ��̽��� ����� ���� ��ȸ ��û
		Database::Task task;
		task.Socket = targetSocket;
		task.MessageType = protocol::MessageContent_REQUEST_CONNECT;
		task.DatabaseName = "User";
		task.ProcedureName = "CheckAndAddAccount";
		task.Parameters = userId;
		return task;
	}

	void CreateResponseConnect(bool loginSuccess, const std::string& authToken, int lobbyPort, uint32_t& contentsType, std::string& buffer, int& bodySize)
	{
		flatbuffers::FlatBufferBuilder builder;

		auto authTokenOffset = builder.CreateString(authToken);
		auto responseConnect = protocol::CreateRESPONSE_CONNECT(builder, loginSuccess, authTokenOffset, lobbyPort);

		builder.Finish(responseConnect);

		contentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);
		bodySize = builder.GetSize();
		buffer.assign(reinterpret_cast<const char*>(builder.GetBufferPointer()), bodySize);
	}

}