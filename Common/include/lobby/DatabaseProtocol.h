#pragma once
#include <memory>
#include <string>
#include <vector>

#define NOMINMAX 
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

namespace Common
{
	namespace Protocol
	{
		//프로시저 리절트 프로토콜.

		//struct InventoryItem
		//{
		//	GUID Guid;
		//	LONGLONG ItemSeed;
		//	int ItemCount;
		//};

		struct BasicResult
		{
			uint32_t ContentsType;
		};

		struct ResultUserLogIn : public BasicResult
		{
			uint64_t AccountNumber;
			uint32_t Detail;
			bool Success;
		};

		struct ResultGetAccountData : public BasicResult
		{
			uint64_t AccountNumber;
			std::string AccountUID;
			uint64_t GameMoney;
			int GameMoneyRank;
			int InventoryCapacity;
			int Success;
		};

		struct InventorySlotData 
		{
			std::string GuidStr;
			long ItemSeed;
			int ItemCount;
		};

		struct ResultGetInventoryData : public BasicResult
		{
			std::vector<InventorySlotData> InventoryItems;
		};

		void SetProcedureResult(ResultUserLogIn& resultUserLogIn, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultGetAccountData& resultGetAccountData, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultGetInventoryData& resultGetInventoryData, SQLHSTMT& hstmt);
	}
}