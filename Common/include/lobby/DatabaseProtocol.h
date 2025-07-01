#pragma once
#include <memory>
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
			uint32_t Detail;
			bool Success;
		public:
			void SetProcedureResult(SQLHSTMT& hstmt) override;
		};
	}
}