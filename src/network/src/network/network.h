#pragma once

#include "definitions.h"

namespace Cjing3D
{
namespace Network
{
    enum class ErrorCode : U8
    {
        SUCCESS = 0,
        FAIL_SEND = 1,
        TIMEOUT = 2,
        SERVER_FAIL = 3,
        KILLING_THREADS = 4
    };

    enum class ConnectionStatus : U8
    {
        Undefined,
        Connecting,
        Connected,
        Closing,
        Closed
    };

    struct EndPoint
    {
        StaticString<32> mHost;
        U16 mPort;
    };

    class Message
    {
    private:
        MemoryStream mStream;
        EndPoint mEndPoint;
        ErrorCode mErrorCode;

    public:
    };
    using MessagePtr = SharedPtr<Message>;

    class Connection
    {
    public:
        Connection() {}
        virtual ~Connection() {}

        virtual void Disconnect() = 0;
        virtual void Send() = 0;
        virtual void Receive() = 0;
        virtual ConnectionStatus GetStatus()const = 0;
        virtual bool IsConnected()const = 0;
    };

    class ClientInterface
    {
    public:
        virtual ~ClientInterface() {}
        virtual bool Connect(const String& address, I32 port) = 0;
        virtual void Disconnect() = 0;
        virtual bool IsConnected()const = 0;
    };

    class ServerInterface
    {
    public:
        virtual ~ServerInterface() {}
        virtual bool Start() = 0;
        virtual void Update() = 0;
        virtual void Stop() = 0;
        virtual bool OnClientConnect(SharedPtr<Connection> clientConn) { return true; }
    };
}
}