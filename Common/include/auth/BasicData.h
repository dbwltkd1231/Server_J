#pragma once

namespace Common
{
	namespace Auth
	{
		struct BasicData
		{
		public:
			BasicData() = default;
			~BasicData() = default;
			BasicData(const BasicData& other) : ContentsType(other.ContentsType) {}

		public:
			uint32_t ContentsType;
			bool Success;
		};

		struct RequestConnectData : public BasicData
		{
		public:
			RequestConnectData() = default;
			~RequestConnectData() = default;
			RequestConnectData(const RequestConnectData& other) : BasicData(other), AccountNumber(other.AccountNumber), AccountUID(other.AccountUID), IsNew(other.IsNew) {}

		public:
			long AccountNumber;
			std::string AccountUID;
			bool IsNew;
		};
	}
}
