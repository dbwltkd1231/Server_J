#pragma once
#include "DatabaseProtocol.h"

#include "../library/flatbuffers/flatbuffers.h"
#include "../include/utility/MESSAGE_PROTOCOL_generated.h"

namespace Common
{
	namespace Protocol
	{

		ResultUserLogIn::ResultUserLogIn() = default;
		ResultUserLogIn::~ResultUserLogIn() = default;
		ResultUserLogIn::ResultUserLogIn(const ResultUserLogIn& other) : BasicData(other), Detail(other.Detail) {}

		void ResultUserLogIn::SetProcedureResult(SQLHSTMT& hstmt)
		{
			this->ContentsType = static_cast<uint32_t>(protocol::MessageContent_RESPONSE_LOGIN);

			int feedback = -1;

			SQLLEN cbFeedback = 0;

			SQLBindCol(hstmt, 1, SQL_C_LONG, &feedback, 0, &cbFeedback);

			if (SQLFetch(hstmt) == SQL_SUCCESS || SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
			{
				this->Detail = (protocol::FEEDBACK_LOGIN)feedback;
			}
			else
			{
				this->Detail = protocol::FEEDBACK_LOGIN::FEEDBACK_LOGIN_BEGIN;
			}
		}





	}
}