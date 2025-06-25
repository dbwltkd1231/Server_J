#pragma once
#include <winsock2.h>
#include <memory>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/game/LOGINSERVER_PROTOCOL_generated.h"

#include "../database/Task.h"
#include "../game/BasicData.h"
#include "../utility/Debug.h"


namespace Game
{
    std::shared_ptr<Game::BasicData> GetAccountCheckData(ULONG_PTR& targetSocket, SQLHSTMT& hstmt);

    std::shared_ptr<Game::BasicData> GetSqlData(ULONG_PTR& targetSocket, int contentsType, SQLHSTMT& hstmt)
    {
        auto messageType = static_cast<protocol::MessageContent>(contentsType);

        switch (messageType)
        {
            case protocol::MessageContent_REQUEST_CONNECT:
            {
                auto result = GetAccountCheckData(targetSocket, hstmt);
                return result;
            }
        }

        return nullptr;
    }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////


    // SQL������ REQUEST_CONNECT ��û�� ���� Task��ü ����
    Database::Task CreateQuerryAccountCheck(ULONG_PTR targetSocket, std::string userId)
    {
        // �α��� ��û ó�� ����
        // ��: �����ͺ��̽��� ����� ���� ��ȸ ��û
        Database::Task task;
        task.SocketPtr = targetSocket;
        task.MessageType = protocol::MessageContent_REQUEST_CONNECT;
        task.DatabaseName = Database::DatabaseType::User;
        task.ProcedureName = "CheckAndAddAccount";
        task.Parameters = "'" + userId + "'";
        return task;
    }
    //REQUEST_CONNECT ��û�� ���� SQL ��������� ��� ��ȯ.
    std::shared_ptr<Game::BasicData> GetAccountCheckData(ULONG_PTR& targetSocket, SQLHSTMT& hstmt)
    {
        Game::RequestConnectData requestData;
        requestData.ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);

        char accountUIDBuffer[56] = { 0 };  // VARCHAR(55) + null
        int isNew = -1;

        SQLLEN cbResultCode = 0;
        SQLLEN cbAccountUID = 0;

        // ù ��° �÷�: AccountUID
        SQLBindCol(hstmt, 1, SQL_C_CHAR, accountUIDBuffer, sizeof(accountUIDBuffer), &cbAccountUID);
        // �� ��° �÷�: AccountExists or AccountAdded (int)
        SQLBindCol(hstmt, 2, SQL_C_LONG, &isNew, 0, &cbResultCode);

        if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
        {
            requestData.UID = std::string(accountUIDBuffer, cbAccountUID);  // ���� �־��ִ� �ʵ��� ����
            requestData.IsNew = (isNew != 1);  // 1: �̹� ���� (��ȸ ����), 0: ���� ������
        }
        else
        {
            requestData.IsNew = false;
            requestData.UID = "";
        }

        std::shared_ptr<Game::RequestConnectData> requestDataPtr = std::make_shared<Game::RequestConnectData>(requestData);
        return std::static_pointer_cast<Game::BasicData>(requestDataPtr);
    }
}