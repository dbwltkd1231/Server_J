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

		struct GameItem
		{
			long ItemSeed;
			bool IsPile;
			int PileCountMax;
			int BreakMoneyAmount;
		};

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

		struct ResultAddInventoryItem : public BasicResult
		{
			std::vector<InventorySlotData> UpdateInventoryDataSet;
		};

		struct ResultBreakInventoryItem : public BasicResult
		{
			int Success;
			std::string GuidStr;
			int MoneyReward;
			int RemoveCount;
		};

		void SetProcedureResult(std::vector<GameItem>& gameItemSet, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultUserLogIn& resultUserLogIn, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultGetAccountData& resultGetAccountData, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultGetInventoryData& resultGetInventoryData, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultAddInventoryItem& resultAddInventoryItem, SQLHSTMT& hstmt);
		void SetProcedureResult(ResultBreakInventoryItem& resultBreakInventoryItem, SQLHSTMT& hstmt);
	}
}