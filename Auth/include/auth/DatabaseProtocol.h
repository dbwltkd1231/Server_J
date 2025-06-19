#pragma once
#include <winsock2.h>
#include <memory>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/database/LOGINSERVER_PROTOCOL_generated.h"
#include "../auth/BasicData.h"
#include "../utility/Debug.h"


namespace Auth
{
    std::shared_ptr<Auth::BasicData> ReadRequestConnect(ULONG_PTR& targetSocket, SQLHSTMT& hstmt);

	std::shared_ptr<Auth::BasicData> ReadReturnData(ULONG_PTR& targetSocket, int contentsType, SQLHSTMT& hstmt)
	{
		auto messageType = static_cast<protocol::MessageContent>(contentsType);

		switch (messageType)
		{
			case protocol::MessageContent_RESPONSE_CONNECT:
			{
				auto result = ReadRequestConnect(targetSocket, hstmt);
                return result;
			}
		}

	}

    std::shared_ptr<Auth::BasicData> ReadRequestConnect(ULONG_PTR& targetSocket, SQLHSTMT& hstmt)
	{
        Auth::RequestConnectData requestData;
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

        std::shared_ptr<Auth::BasicData> basicData = std::make_shared<Auth::BasicData>(requestData);
        return basicData;
	}
}