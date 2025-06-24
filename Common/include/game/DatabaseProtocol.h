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
            case protocol::MessageContent_RESPONSE_CONNECT:
            {
            auto result = GetAccountCheckData(targetSocket, hstmt);
            return result;
            }
        }

    }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////


    // SQL서버에 REQUEST_CONNECT 요청을 위한 Task객체 생성
    Database::Task CreateRequestConnect(ULONG_PTR targetSocket, std::string userId)
    {
        // 로그인 요청 처리 로직
        // 예: 데이터베이스에 사용자 정보 조회 요청
        Database::Task task;
        task.Socket = targetSocket;
        task.MessageType = protocol::MessageContent_REQUEST_CONNECT;
        task.DatabaseName = "User";
        task.ProcedureName = "CheckAndAddAccount";
        task.Parameters = userId;
        return task;
    }

    //REQUEST_CONNECT 요청에 대한 SQL 쿼리실행및 결과 반환.
    std::shared_ptr<Game::BasicData> GetAccountCheckData(ULONG_PTR& targetSocket, SQLHSTMT& hstmt)
	{
        Game::RequestConnectData requestData;
		requestData.ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);

        int resultCode = -1;
        SQLLEN cbResult = 0;

        SQLBindCol(hstmt, 1, SQL_C_LONG, &resultCode, 0, &cbResult);
        if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
        {
			requestData.IsCreate = resultCode == 0 ? true : false; // 0이면 계정 조회 실패 -> 생성, 1이면 계정 조회 성공.
            requestData.IsSuccess = true;
        }
        else
        {
            requestData.IsSuccess = false;
        }

        std::shared_ptr<Game::BasicData> basicData = std::make_shared<Game::BasicData>(requestData);
        return basicData;
	}
}