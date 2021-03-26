#pragma once

#include "definitions.h"
#include "core\concurrency\concurrentQueue.h"
#include "core\signal\listener.h"

namespace Cjing3D
{
namespace Network
{
    static size_t constexpr DEFAULT_TCP_BUFFER_SIZE = 1536;

    enum class NetEvent : U8
    {
        CONNECT,
        DISCONNECT,
        START,
        STOP,
        RECEIVE,

        COUNT
    };

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

    class NetMessage
    {
    private:
        MemoryStream mStream;
        EndPoint mEndPoint;
        ErrorCode mErrorCode;

    public:
    };
    using MessagePtr = SharedPtr<NetMessage>;

    class NetConnection : public ENABLE_SHARED_FROM_THIS<NetConnection>
    {
    public:
        NetConnection() {}
        virtual ~NetConnection() {}

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
        virtual bool OnClientConnect(SharedPtr<NetConnection> clientConn) { return true; }
        virtual bool IsStarted()const = 0;
    };
}
}