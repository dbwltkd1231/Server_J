#pragma once
#include "Session.h"
#include "Debug.h"

namespace Network
{
	Session::Session()
	{
		mActive = false;
		_acceptCallback = nullptr;
	}

	Session::~Session()
	{
		mActive = false;
		_acceptCallback = nullptr;
	}

	void Session::Initialize(
		std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket)> acceptCallback,
		std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket)> receiveCallback,
		std::function<void(CustomOverlapped* overlappedPtr, ULONG_PTR socket, int bytesTransferred, int errorCode)> disconnectCallback,
		std::function<void(CustomOverlapped* overlappedPtr)> sendCallback
	)
	{
		_acceptCallback = acceptCallback;
		_receiveCallback = receiveCallback;
		_disconnectCallback = disconnectCallback;
		_sendCallback = sendCallback;

		Utility::Log("Network", "Session", "Initialize !");
	}

	void Session::Activate(HANDLE mIocpHandle)
	{
		mActive = true;

		std::thread thread([this, mIocpHandle]() { this->Process(mIocpHandle); });
		thread.detach();

		Utility::Log("Network", "Session", "Activate !");
	}

	void Session::Deactivate()
	{
		mActive = false;
		Utility::Log("Network", "Session", "Deactivate !");
	}

	void Session::Process(HANDLE mIocpHandle)
	{
		CustomOverlapped* overlapped = nullptr;
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;

		while (mActive)
		{
			BOOL result = GetQueuedCompletionStatus(mIocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

			if (!result)
			{
				int errorCode = WSAGetLastError();
				Utility::LogError("Network", "Session", "GetQueuedCompletionStatus Failed! Error Code: " + std::to_string(errorCode));

				switch (errorCode)
				{
				case WSAECONNRESET:
				case WSAECONNABORTED:
				case WSAENETRESET:
				case WSAETIMEDOUT:
				case WSAENOTCONN:
				case WSAESHUTDOWN:
				case ERROR_NETNAME_DELETED:
					_disconnectCallback(overlapped, completionKey, bytesTransferred, errorCode);
					break;
				default:
					_receiveCallback(overlapped, completionKey);
					break;
				}

				continue;
			}
			Utility::Log("Network", "Session", "Message Recieve Check !");

			switch (overlapped->GetOperation())
			{
			case OperationType::OP_ACCEPT:
			{
				if (overlapped->GetSocket() == nullptr)
				{
					Utility::LogError("Network", "Session", "Socket is null !");
					continue;
				}

				_acceptCallback(overlapped, (ULONG_PTR)overlapped->GetSocket().get());
				break;
			}
			case OperationType::OP_RECV:
			{
				if (bytesTransferred <= 0)
				{
					_disconnectCallback(overlapped, completionKey, bytesTransferred, 0);
					continue;
				}

				_receiveCallback(overlapped, completionKey);
				break;
			}
			case OperationType::OP_SEND:
			{
				_sendCallback(overlapped);
				break;
			}
			case OperationType::OP_DEFAULT:
			{

				break;
			}
			}
		}
	}
}