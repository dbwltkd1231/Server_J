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
			std::function<void(ULONG_PTR socket)> acceptCallback,
			std::function<void(ULONG_PTR socket, CustomOverlapped* overlappedPtr)> receiveCallback,
			std::function<void(ULONG_PTR socket, int bytesTransferred, int errorCode)> disconnectCallback
		);
	public:
		void Activate(HANDLE mIocpHandle);
		void Process(HANDLE iocpHandle);
		void Deactivate();

	private:
		std::function<void(ULONG_PTR socket)> _acceptCallback;
		std::function<void(ULONG_PTR socket, CustomOverlapped* overlappedPtr)> _receiveCallback;
		std::function<void(ULONG_PTR socket, int bytesTransferred, int errorCode)> _disconnectCallback;

		bool mActive;
	};
}