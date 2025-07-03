#pragma once
#include <iostream>
#include <memory>
#include <functional>
#include <thread>
#include <winsock2.h>

#include "CustomOverlapped.h"

namespace Network
{
	class Session
	{
	public:
		Session();
		~Session();
		void Initialize
		(
			std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket)> acceptCallback,
			std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket)> receiveCallback,
			std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket, int bytesTransferred, int errorCode)> disconnectCallback,
			std::function<void(CustomOverlapped* overlappedPtr)> sendCallback
		);
	public:
		void Activate(HANDLE mIocpHandle);
		void Process(HANDLE iocpHandle);
		void Deactivate();

	private:
		std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket)> _acceptCallback;
		std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket)> _receiveCallback;
		std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket, int bytesTransferred, int errorCode)> _disconnectCallback;
		std::function<void(CustomOverlapped* overlappedPtr)> _sendCallback;
		bool mActive;
	};
}