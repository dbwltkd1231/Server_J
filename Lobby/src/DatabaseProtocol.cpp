#pragma once

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

		ResultUserLogIn::ResultUserLogIn() = default;
		ResultUserLogIn::~ResultUserLogIn() = default;
		ResultUserLogIn::ResultUserLogIn(const ResultUserLogIn& other)
			: BasicData(other), AccountNumber(other.AccountNumber), Detail(other.Detail), Success(other.Success)
		{
		}

		void ResultUserLogIn::SetProcedureResult(SQLHSTMT& hstmt)
		{
			this->ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_LOGIN);

			long accountNumber = 0;
			int feedback = -1;

			SQLLEN lenAccountNumber = 0;
			SQLLEN lenFeedback = 0;

			SQLBindCol(hstmt, 1, SQL_C_LONG, &accountNumber, 0, &lenAccountNumber);
			SQLBindCol(hstmt, 2, SQL_C_LONG, &feedback, 0, &lenFeedback);
			if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
			{
				this->AccountNumber = accountNumber;
				this->Detail = (protocol::FEEDBACK_LOGIN)feedback;
				this->Success = (this->Detail == protocol::FEEDBACK_LOGIN::FEEDBACK_LOGIN_Success);
			}
			else
			{
				this->Detail = protocol::FEEDBACK_LOGIN::FEEDBACK_LOGIN_BEGIN;
			}
		}

		//////////////////////////////////////////////////////////

		ResultGetAccountData::ResultGetAccountData() = default;
		ResultGetAccountData::~ResultGetAccountData() = default;

		ResultGetAccountData::ResultGetAccountData(const ResultGetAccountData& other)
			: BasicData(other),
			AccountNumber(other.AccountNumber),
			AccountUID(other.AccountUID),
			GameMoney(other.GameMoney),
			GameMoneyRank(other.GameMoneyRank),
			InventoryCapacity(other.InventoryCapacity),
			Success(other.Success)
		{
		}

		void ResultGetAccountData::SetProcedureResult(SQLHSTMT& hstmt)
		{
			this->ContentsType = static_cast<uint32_t>(protocol::MessageContent_NOTICE_ACCOUNT);

			long accountNumber = 0;
			SQLCHAR  accountUID[56];
			long gameMoney = 0;
			int gameMoneyRank = 0;
			int inventoryCapacity = 0;
			int success = 0;

			SQLLEN lenAccountNumber = 0;
			SQLLEN lenAccountUID = 0;
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

				this->AccountNumber = accountNumber;
				this->AccountUID = idString;
				this->GameMoney = gameMoney;
				this->GameMoneyRank = gameMoneyRank;
				this->InventoryCapacity = inventoryCapacity;
				this->Success = success;
			}
			else
			{
				this->Success = false;
			}
		}
	}
}