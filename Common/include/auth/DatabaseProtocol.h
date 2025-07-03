#pragma once
#include <winsock2.h>
#include <memory>

#define NOMINMAX 
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "../library/flatbuffers/flatbuffers.h"
#include "MESSAGE_PROTOCOL_generated.h"

#include "../database/Task.h"
#include "../auth/BasicData.h"
#include "../utility/Debug.h"

namespace Common
{
    namespace Auth
    {
        std::shared_ptr<Common::Auth::BasicData> GetAccountCheckData(ULONG_PTR& targetSocket, SQLHSTMT& hstmt);

        std::shared_ptr<Auth::BasicData> GetSqlData(ULONG_PTR& targetSocket, int contentsType, SQLHSTMT& hstmt)
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


        // SQL서버에 REQUEST_CONNECT 요청을 위한 Task객체 생성
        Database::Task CreateQuerryAccountCheck(ULONG_PTR targetSocket, std::string userId)
        {
            // 로그인 요청 처리 로직
            // 예: 데이터베이스에 사용자 정보 조회 요청
            Database::Task task;
            task.SocketPtr = targetSocket;
            task.QueryType = Database::DatabaseQueryType::CheckAndAddAccount;
            task.NetworkType = protocol::MessageContent_REQUEST_CONNECT;
            task.DatabaseName = Database::DatabaseType::User;
            task.ProcedureName = "CheckAndAddAccount";
            task.Parameters = " '" + userId + "'";
            return task;
        }

        //REQUEST_CONNECT 요청에 대한 SQL 쿼리실행및 결과 반환.
        std::shared_ptr<Auth::BasicData> GetAccountCheckData(ULONG_PTR& targetSocket, SQLHSTMT& hstmt)
        {
            Auth::RequestConnectData requestData;
            requestData.ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_CONNECT);

            long accountNumber = 0;
            char accountUIDBuffer[56] = { 0 };  // VARCHAR(55) + null
            int isNew = -1;

            SQLLEN cbAccountNumber = 0;
            SQLLEN cbAccountUID = 0;
            SQLLEN cbResultCode = 0;
   
            SQLBindCol(hstmt, 1, SQL_C_LONG, &accountNumber, 0, &cbAccountNumber);
            SQLBindCol(hstmt, 2, SQL_C_CHAR, accountUIDBuffer, sizeof(accountUIDBuffer), &cbAccountUID);
            SQLBindCol(hstmt, 3, SQL_C_LONG, &isNew, 0, &cbResultCode);

            if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
            {
                requestData.Success = true;
                requestData.AccountNumber = accountNumber;
                requestData.AccountUID = std::string(accountUIDBuffer, cbAccountUID);  // 새로 넣어주는 필드라고 가정
                requestData.IsNew = (isNew != 1);  // 1: 이미 존재 (조회 성공), 0: 새로 생성됨
            }
            else
            {
                requestData.Success = false;
            }

            std::shared_ptr<Auth::RequestConnectData> requestDataPtr = std::make_shared<Auth::RequestConnectData>(requestData);
            return std::static_pointer_cast<Auth::BasicData>(requestDataPtr);
        }
    }
}