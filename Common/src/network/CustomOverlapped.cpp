#pragma once

#include "CustomOverlapped.h"
#include "ConstValue.h"

namespace Network
{
    CustomOverlapped::CustomOverlapped()
    {
        Wsabuf[0].buf = new char[sizeof(MessageHeader)];
        Wsabuf[0].len = sizeof(MessageHeader);

        Wsabuf[1].buf = new char[Utility::BUFFER_SIZE_MAX];
        Wsabuf[1].len = Utility::BUFFER_SIZE_MAX;

        SocketPtr = nullptr;
        Operation = OperationType::OP_DEFAULT;
        this->hEvent = NULL;
    }

    CustomOverlapped::~CustomOverlapped()
    {
        SocketPtr = nullptr;

        delete[] Wsabuf[0].buf;
        Wsabuf[0].len = 0;

        delete[] Wsabuf[1].buf;
        Wsabuf[1].len = 0;

        Operation = OperationType::OP_DEFAULT;
        this->hEvent = NULL;
    }

    CustomOverlapped::CustomOverlapped(const CustomOverlapped& other)
    {
        if (other.Wsabuf[0].len > 0)
        {
            Wsabuf[0].buf = other.Wsabuf[0].buf;
            Wsabuf[0].len = other.Wsabuf[0].len;
        }
        else
        {
            Wsabuf[0].buf = nullptr;
            Wsabuf[0].len = 0;
        }

        if (other.Wsabuf[1].len > 0)
        {
            Wsabuf[1].buf = other.Wsabuf[1].buf;
            Wsabuf[1].len = other.Wsabuf[1].len;
        }
        else
        {
            Wsabuf[1].buf = nullptr;
            Wsabuf[1].len = 0;
        }

        SocketPtr = other.SocketPtr;
        this->hEvent = other.hEvent;
        Operation = other.Operation;
    }

    std::shared_ptr<SOCKET> CustomOverlapped::GetSocket() const
    {
        return SocketPtr;
    }

    OperationType CustomOverlapped::GetOperation() const
    {
        return Operation;
    }

    void CustomOverlapped::SetSocket(std::shared_ptr<SOCKET> socketPtr)
    {
        SocketPtr = socketPtr;
    }

    void CustomOverlapped::SetHeader(const MessageHeader& headerData)
    {
        memset(Wsabuf[0].buf, 0, Wsabuf[0].len);

        auto header = new MessageHeader(headerData);
        Wsabuf[0].buf = reinterpret_cast<char*>(header);
        Wsabuf[0].len = sizeof(MessageHeader);
    }

    void CustomOverlapped::SetBody(const char* bodyBuffer, ULONG bodyLen)
    {
        memset(Wsabuf[1].buf, 0, bodyLen);

        if (bodyLen > Utility::BUFFER_SIZE_MAX)
        {
            Utility::Log("Network", "CustomOverlapped", " 메세지의 body가 BUFFER_SIZE보다 큼!!");
            return;
        }

        memcpy(Wsabuf[1].buf, bodyBuffer, bodyLen);
        Wsabuf[1].len = bodyLen;

    }

    void CustomOverlapped::SetOperationType(OperationType operation)
    {
        Operation = operation;
    }

    void CustomOverlapped::Clear()
    {
        memset(Wsabuf[0].buf, 0, sizeof(MessageHeader));
        Wsabuf[0].len = sizeof(MessageHeader);
        memset(Wsabuf[1].buf, 0, Wsabuf[1].len);
        Wsabuf[1].len = Utility::BUFFER_SIZE_MAX;

        Operation = OperationType::OP_DEFAULT;
        SocketPtr = nullptr;
        this->hEvent = NULL;
    }
}