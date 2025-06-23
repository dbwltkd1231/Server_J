#pragma once

namespace Game
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