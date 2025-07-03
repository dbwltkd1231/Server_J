#pragma once
#include <string>
#include <cstdio> 
#include "DatabaseProtocol.h"

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"

namespace Common
{
	namespace Protocol
	{
		std::string WstringToUTF8(const std::wstring& wstr)
		{
			if (wstr.empty()) return std::string();

			int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
			std::string strTo(sizeNeeded, 0);
			WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], sizeNeeded, NULL, NULL);

			return strTo;
		}

		inline std::string GuidToString(const GUID& guid)
		{
			char buffer[40]; // 36자 + null terminator 여유
			sprintf_s(
				buffer, sizeof(buffer),
				"%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1],
				guid.Data4[2], guid.Data4[3], guid.Data4[4],
				guid.Data4[5], guid.Data4[6], guid.Data4[7]
			);
			return std::string(buffer);
		}

		void SetProcedureResult(ResultUserLogIn& resultUserLogIn, SQLHSTMT& hstmt)
		{
			long accountNumber = 0;
			int feedback = -1;

			SQLLEN lenAccountNumber = 0;
			SQLLEN lenFeedback = 0;

			SQLBindCol(hstmt, 1, SQL_C_LONG, &accountNumber, 0, &lenAccountNumber);
			SQLBindCol(hstmt, 2, SQL_C_LONG, &feedback, 0, &lenFeedback);
			if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
			{
				resultUserLogIn.AccountNumber = accountNumber;
				resultUserLogIn.Detail = (protocol::FEEDBACK_LOGIN)feedback;
				resultUserLogIn.Success = (resultUserLogIn.Detail == protocol::FEEDBACK_LOGIN::FEEDBACK_LOGIN_Success);
			}
			else
			{
				resultUserLogIn.Detail = protocol::FEEDBACK_LOGIN::FEEDBACK_LOGIN_BEGIN;
			}
		}

		void SetProcedureResult(ResultGetAccountData& resultGetAccountData, SQLHSTMT& hstmt)
		{
			long accountNumber = 0;
			SQLCHAR  accountUID[56];
			long gameMoney = 0;
			int gameMoneyRank = 0;
			int inventoryCapacity = 0;
			int success = 0;

			SQLLEN lenAccountNumber = 0; // 정수형타입에는 안넣어주어도 문제는 안생기는것으로 확인되지만,
			SQLLEN lenAccountUID = 0; // SQLCHAR타입에는 꼭 넣어주어야 한다.
			SQLLEN lenGameMoney = 0;
			SQLLEN lenGameMoneyRank = 0;
			SQLLEN lenInventoryCapacity = 0;
			SQLLEN lenSuccess = 0;

			SQLBindCol(hstmt, 1, SQL_C_LONG, &accountNumber, 0, &lenAccountNumber);
			SQLBindCol(hstmt, 2, SQL_C_CHAR, &accountUID, sizeof(accountUID), &lenAccountUID);
			SQLBindCol(hstmt, 3, SQL_C_LONG, &gameMoney, 0, &lenGameMoney);
			SQLBindCol(hstmt, 4, SQL_C_LONG, &gameMoneyRank, 0, &lenGameMoneyRank);
			SQLBindCol(hstmt, 5, SQL_C_LONG, &inventoryCapacity, 0, &lenInventoryCapacity);
			SQLBindCol(hstmt, 6, SQL_C_LONG, &success, 0, &lenSuccess);

			if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
			{
				std::string idString(reinterpret_cast<char*>(accountUID));

				resultGetAccountData.AccountNumber = accountNumber;
				resultGetAccountData.AccountUID = idString;
				resultGetAccountData.GameMoney = gameMoney;
				resultGetAccountData.GameMoneyRank = gameMoneyRank;
				resultGetAccountData.InventoryCapacity = inventoryCapacity;
				resultGetAccountData.Success = success;
			}
			else
			{
				resultGetAccountData.Success = false;
			}
		}

		void SetProcedureResult(ResultGetInventoryData& result, SQLHSTMT& hstmt)
		{
			GUID guid;
			long itemSeed;
			int itemCount;

			SQLLEN lenGuid = 0;
			SQLLEN lenItemSeed = 0;
			SQLLEN lenItemCount = 0;

			SQLBindCol(hstmt, 1, SQL_C_GUID, &guid, sizeof(GUID), &lenGuid);
			SQLBindCol(hstmt, 2, SQL_C_LONG, &itemSeed, 0, &lenItemSeed);
			SQLBindCol(hstmt, 3, SQL_C_LONG, &itemCount, 0, &lenItemCount);

			result.InventoryItems.clear();

			while (SQLFetch(hstmt) == SQL_SUCCESS)
			{
				std::string guidString = GuidToString(guid);

				result.InventoryItems.push_back(InventorySlotData{ guidString, itemSeed, itemCount });
			}
		}
	}
}