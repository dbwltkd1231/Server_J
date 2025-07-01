#pragma once
#include <memory>
#include <string>

#define NOMINMAX 
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

namespace Common
{
	namespace Protocol
	{
		class BasicData
		{
		public:
			BasicData() = default;
			virtual ~BasicData() = default;
			BasicData(const BasicData& other) : ContentsType(other.ContentsType) {}

		public:
			uint32_t ContentsType;

		public:
			virtual void SetProcedureResult(SQLHSTMT& hstmt) = 0;
		};

		class ResultUserLogIn : public BasicData
		{
		public:
			ResultUserLogIn();
			~ResultUserLogIn();
			ResultUserLogIn(const ResultUserLogIn& other);

		public:
			uint64_t AccountNumber;
			uint32_t Detail;
			bool Success;
		public:
			void SetProcedureResult(SQLHSTMT& hstmt) override;
		};

		class ResultGetAccountData : public BasicData
		{
		public:
			ResultGetAccountData();
			~ResultGetAccountData();
			ResultGetAccountData(const ResultGetAccountData& other);

		public:
			uint64_t AccountNumber;
			std::string AccountUID;
			uint64_t GameMoney;
			int GameMoneyRank;
			int InventoryCapacity;
			int Success;
		public:
			void SetProcedureResult(SQLHSTMT& hstmt) override;
		};
	}
}