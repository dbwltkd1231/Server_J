#pragma once
#include <memory>

#define NOMINMAX
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>

namespace Network
{
    enum OperationType
    {
        OP_DEFAULT = 0,
        OP_ACCEPT = 1,
        OP_RECV = 2,
        OP_SEND = 3,
    };

    struct MessageHeader
    {
        uint32_t BodySize;
        uint32_t ContentsType;

        MessageHeader(uint32_t bodySize, uint32_t contentsType) : BodySize(bodySize), ContentsType(contentsType)
        {
        }

        MessageHeader(const MessageHeader& other) : BodySize(other.BodySize), ContentsType(other.ContentsType)
        {
        }
    };

    struct CustomOverlapped :OVERLAPPED
    {
    public:
        CustomOverlapped();
        ~CustomOverlapped();
        CustomOverlapped(const CustomOverlapped& other);

    public:
        WSABUF Wsabuf[2];
        std::shared_ptr<SOCKET> SocketPtr;
        OperationType Operation;

    public:
        std::shared_ptr<SOCKET> GetSocket() const;
        OperationType GetOperation() const;

    public:
        void SetHeader(const MessageHeader& headerData);
        void SetBody(const char* bodyBuffer, ULONG bodyLen);
        void SetSocket(std::shared_ptr<SOCKET> socketPtr);
        void SetOperationType(OperationType operation);
        void Clear();

    };
}