#pragma once

namespace Auth
{
	struct BasicData
	{
		uint32_t ContentsType;
	};

	struct RequestConnectData : public BasicData
	{
		bool IsCreate;
		bool IsSuccess;
	};
}