#pragma once
#include "network\network.h"
#include "core\container\dynamicArray.h"
#include "core\helper\stream.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    //////////////////////////////////////////////////////////////////////////
    // IOContext
    //////////////////////////////////////////////////////////////////////////
    class IOContextASIO
    {
    public:
        IOContextASIO();
        ~IOContextASIO();
        IOContextASIO(const IOContextASIO& rhs) = delete;
        void operator=(const IOContextASIO& rhs) = delete;

        bool Start();
        void Stop();
        void Wait();

        asio::io_context& GetContext() {
            return mContext;
        }
        asio::io_context::strand& GetStrand() {
            return mStrand;
        }
        bool IsStoped()const {
            return mIsStoped;
        }

    private:
        asio::io_context mContext;
        Concurrency::Thread mThreadContext;
        asio::executor_work_guard<asio::io_context::executor_type>* mWork = nullptr;
        bool mIsStoped = true;

        // io_context::strand class provides the ability to post and 
        // dispatch handlers with the guarantee that none of those handlers
        // will execute concurrently.
        asio::io_context::strand mStrand;
    };

    //////////////////////////////////////////////////////////////////////////
    // Connectoin
    //////////////////////////////////////////////////////////////////////////
    class ConnectionAsio : public NetConnection
    {
    public:
        ConnectionAsio(IOContextASIO& context, asio::ip::tcp::socket socket, EventListener<(size_t)NetEvent::COUNT>& listener);
        virtual ~ConnectionAsio();

        void Disconnect()override;
        void Send()override;
        void Receive()override;
        bool IsConnected()const override;

        void ConnectToClient();
        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endPoints);
        void PostAsyncRead();

        ConnectionStatus GetStatus()const override {
            return mStatus;
        }

    private:
        void PostDisconnect();
        void HandleConnect(asio::error_code ec, asio::ip::tcp::endpoint endpoint);
        void HandleRead(const asio::error_code& ec, std::size_t bytesReceived);

    private:
        // common
        IOContextASIO& mContext;
        asio::ip::tcp::socket mSocket;
        volatile ConnectionStatus mStatus;
        MemoryStream mRecvBuffer;
        EventListener<(size_t)NetEvent::COUNT>& mListener;
        
        // client
        Concurrency::AtomicFlag mFireConnectFlag;
    };

    //////////////////////////////////////////////////////////////////////////
    // ClientInterface
    //////////////////////////////////////////////////////////////////////////
    class ClientInterfaceASIO : public ClientInterface
    {
    public:
        ClientInterfaceASIO();
        virtual ~ClientInterfaceASIO();

        bool Connect(const String& address, I32 port)override;
        void Disconnect()override;
        bool IsConnected()const override;

    private:
        IOContextASIO mContext;
        UniquePtr<ConnectionAsio> mConnection;
        EventListener<(size_t)NetEvent::COUNT> mListener;
        String mHostAddress;
        I32 mHostPort = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    // ServerInterface
    //////////////////////////////////////////////////////////////////////////
    class ServerInterfaceASIO : public ServerInterface
    {
    public:
        ServerInterfaceASIO(I32 port);
        virtual ~ServerInterfaceASIO();

        bool Start()override;
        void Update()override;
        void Stop()override;
        bool IsStarted()const override;

        // Receive format: void(SharedPtr<ConnectionAsio>&, Span<const char>)
        template<typename F, typename... T>
        void BindReceive(F&& func, T&&...obj)
        {
            mListener.AddObserver(NetEvent::RECEIVE,
                EventObserver<SharedPtr<ConnectionAsio>&, Span<const char>>(
                    std::forward<F>(func), std::forward<T>(obj)...));
        }

    private:
        void PostHandleAccept();
        void HandleAccept(asio::ip::tcp::socket&& socket, std::error_code ec);

    private:
        IOContextASIO mContext;
        asio::ip::tcp::acceptor mAcceptor;
        asio::ip::tcp::endpoint mEndPoint;
        asio::steady_timer mAcceptorTimer;
        bool mIsStarted = false;

        DynamicArray<SharedPtr<ConnectionAsio>> mActiveConnections;
        EventListener<(size_t)NetEvent::COUNT> mListener;
    };
}
}
#endif