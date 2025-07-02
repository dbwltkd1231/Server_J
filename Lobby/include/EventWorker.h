#pragma once
#include <chrono>
#include <basetsd.h>

namespace Lobby
{
	enum EventType
	{
		Default = -1,

		Login,
		HeartBeat,
		ItemGive,

		End
	};

	class EventWorker
	{
	public:
		EventWorker();
		~EventWorker();
		EventWorker(const EventWorker& other);

	private:
		ULONG_PTR _targetSocket;
		std::chrono::steady_clock::time_point _createTime;
		std::chrono::seconds _durationTimeOut;
		EventType _eventType;

	public:
		void Initialize(ULONG_PTR targetSocket, std::chrono::steady_clock::time_point createTime, std::chrono::seconds durationTimeOut, EventType eventType);
		bool TimerCheck(ULONG_PTR& targetSocket, EventType& eventType);
	};
}