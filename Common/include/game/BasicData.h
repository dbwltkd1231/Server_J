#pragma once

namespace Game
{
	struct BasicData
	{
	public:
		BasicData() = default;
		~BasicData() = default;
		BasicData(const BasicData& other) : ContentsType(other.ContentsType) {}

	public:
		uint32_t ContentsType;
	};

	struct RequestConnectData : public BasicData
	{
	public:
		RequestConnectData() = default;
		~RequestConnectData() = default;
		RequestConnectData(const RequestConnectData& other) : BasicData(other), UID(other.UID), IsNew(other.IsNew) {}

	public:
		std::string UID;
		bool IsNew;
	};
}