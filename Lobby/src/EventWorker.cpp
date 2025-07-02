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
        : _targetSocket(other._targetSocket),
        _createTime(other._createTime),
        _durationTimeOut(other._durationTimeOut),
        _eventType(other._eventType)
    {
    }

	void EventWorker::Initialize(ULONG_PTR targetSocket, std::chrono::steady_clock::time_point createTime, std::chrono::seconds durationTimeOut, EventType eventType)
	{
		_targetSocket = targetSocket;
		_createTime = createTime;
		_durationTimeOut = durationTimeOut;
		_eventType = eventType;
	}

	bool EventWorker::TimerCheck(ULONG_PTR& targetSocket, EventType& eventType)
	{
		bool result = false;
		if (std::chrono::steady_clock::now() - _createTime >= _durationTimeOut) 
		{
			targetSocket = _targetSocket;
			eventType = _eventType;
			result = true;
		}

		return result;
	}
}