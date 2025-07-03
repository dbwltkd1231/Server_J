#include "EventWorker.h"

namespace Lobby
{
	EventWorker::EventWorker()
	{

	}

	EventWorker::~EventWorker()
	{

	}

    EventWorker::EventWorker(const EventWorker& other)
        : TargetSocket(other.TargetSocket),
        _createTime(other._createTime),
        _durationTimeOut(other._durationTimeOut),
		Event(other.Event)
    {
    }

	void EventWorker::Initialize(ULONG_PTR targetSocket, std::chrono::steady_clock::time_point createTime, std::chrono::seconds durationTimeOut, EventType eventType)
	{
		TargetSocket = targetSocket;
		_createTime = createTime;
		_durationTimeOut = durationTimeOut;
		Event = eventType;
	}

	bool EventWorker::TimerCheck()
	{
		bool result = false;
		if (std::chrono::steady_clock::now() - _createTime >= _durationTimeOut) 
		{
			result = true;
		}

		return result;
	}
}