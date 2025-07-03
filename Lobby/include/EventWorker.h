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

	public:
		ULONG_PTR TargetSocket;
		EventType Event;

	private:
		std::chrono::steady_clock::time_point _createTime;
		std::chrono::seconds _durationTimeOut;


	public:
		void Initialize(ULONG_PTR targetSocket, std::chrono::steady_clock::time_point createTime, std::chrono::seconds durationTimeOut, EventType eventType);
		bool TimerCheck();
	};
}