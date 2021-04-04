#pragma once

#include "network\network.h"
#include "network\asio\connectionASIO_TCP.h"
#include "network\asio\networkASIO_impl.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    // TODO：当前对象派生结构及文件太过混乱，需要整理一下
    // 1. 分成Client和server两个文件,改为hpp
  
    //////////////////////////////////////////////////////////////////////////
    // ClientInterface
    //////////////////////////////////////////////////////////////////////////
    class ClientInterfaceASIO : public ClientInterface<ClientInterfaceASIO>
    {
    public:
        ClientInterfaceASIO();
        virtual ~ClientInterfaceASIO();

        bool Connect(const String& address, I32 port);
        void Disconnect();
        bool IsConnected()const ;

        template<typename DataT>
        bool Send(DataT& buffer) {
            return mConnection ? mConnection->Send(buffer) : false;
        }

        // Receive format: void(SharedPtr<ConnectionTCPAsio>&, Span<const char>)
        template<typename F, typename... T>
        void BindReceive(F&& func, T&&...obj)
        {
            mListener.AddObserver(NetEvent::RECEIVE,
                EventObserver<SharedPtr<ConnectionTCPAsio>&, Span<const char>>(
                    std::forward<F>(func), std::forward<T>(obj)...));
        }

    private:
        template<typename TransferCondition>
        bool ConnectImpl(const String& address, I32 port, ConditionWrap<TransferCondition> condition)
        {
            try
            {
                if (!mContext.IsStoped())
                {
                    Logger::Warning("[%s] Connection is already started.", address);
                    return false;
                }

                // record host info
                mHostAddress = address;
                mHostPort = port;

                // Resolve ip-address into tangiable physical address
                asio::ip::tcp::resolver resolver(mContext.GetContext());
                StaticString<16> portStr;
                StringUtils::ToString(port, portStr.toSpan());
                asio::ip::tcp::resolver::results_type endPoints = resolver.resolve(address.c_str(), portStr.c_str());

                // create client connection
                mConnection = CJING_MAKE_SHARED<ConnectionTCPAsio>(mContext, asio::ip::tcp::socket(mContext.GetContext()), mListener);

                // connect to server
                mConnection->ConnectToServer(endPoints, std::move(condition));

                // Launch the asio context in its own thread
                mContext.Start();
            }
            catch (std::exception& e)
            {
                Logger::Error("[Client] Exception:%s", e.what());
                return false;
            }
            return true;
        }

    private:
        IOContextASIO mContext;
        SharedPtr<ConnectionTCPAsio> mConnection;
        EventListener<(size_t)NetEvent::COUNT> mListener;
        String mHostAddress;
        I32 mHostPort = 0;
    };

    //////////////////////////////////////////////////////////////////////////
    // ServerInterface
    //////////////////////////////////////////////////////////////////////////
    class ServerInterfaceASIO :  public ServerInterface<ServerInterfaceASIO>
    {
    public:
        ServerInterfaceASIO();
        virtual ~ServerInterfaceASIO();

        bool Start(const String& address, I32 port);
        void Update();
        void Stop();
        bool IsStarted()const ;

        // Receive format: void(SharedPtr<ConnectionTCPAsio>&, Span<const char>)
        template<typename F, typename... T>
        void BindReceive(F&& func, T&&...obj)
        {
            mListener.AddObserver(NetEvent::RECEIVE,
                EventObserver<SharedPtr<ConnectionTCPAsio>&, Span<const char>>(
                    std::forward<F>(func), std::forward<T>(obj)...));
        }

    private:
        template<typename TransferCondition>
        bool StartImpl(const String& host, I32 port, ConditionWrap<TransferCondition> condition)
        {
            if (IsStarted()) {
                return false;
            }
            try
            {
                // create endPoint
                asio::ip::tcp::endpoint endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);

                // setup acceptor
                mAcceptor.open(endpoint.protocol());
                mAcceptor.bind(endpoint);
                mAcceptor.listen();

                // Launch the asio context in its own thread
                mContext.Start();

                // set flag
                mIsStarted = true;

                // Wait for client connections
                PostHandleAccept(condition);
            }
            catch (std::exception& e)
            {
                Logger::Error("[Server] Exception:%s", e.what());
                return false;
            }

            Logger::Info("[Server] Server started.");
            return true;
        }

        template<typename TransferCondition>
        void PostHandleAccept(ConditionWrap<TransferCondition> condition)
        {
            if (!IsStarted()) {
                return;
            }
            try
            {
                mAcceptor.async_accept(asio::bind_executor(mContext.GetStrand(), [this, condition](std::error_code ec, asio::ip::tcp::socket socket) {
                    HandleAccept(std::move(socket), ec, std::move(condition));
                }));
            }
            catch (std::exception& e)
            {
                Logger::Warning("[Server] Acceptor:%s", e.what());

                // there some exception, do WaitForClientConnection in a seconds
                mAcceptorTimer.expires_after(std::chrono::seconds(1));
                mAcceptorTimer.async_wait(asio::bind_executor(mContext.GetStrand(),
                    [this, condition](const asio::error_code& ec) {
                        if (ec) {
                            return;
                        }
                        asio::post(mContext.GetStrand(), [this, condition]() {
                            PostHandleAccept(std::move(condition));
                        });
                    }));
            }
        }

        template<typename TransferCondition>
        void HandleAccept(asio::ip::tcp::socket&& socket, std::error_code ec, ConditionWrap<TransferCondition> condition)
        {
            // acceptor is closed, just return
            if (ec == asio::error::operation_aborted) {
                return;
            }
            if (ec)
            {
                Logger::Error("[Server] Error connection:%s", ec.message());
                return;
            }
            else
            {
                Logger::Info("[Server] New NetConnectionTCP:%s:%d", socket.remote_endpoint().address().to_string().c_str(), socket.remote_endpoint().port());
                SharedPtr<ConnectionTCPAsio> newConn = CJING_MAKE_SHARED<ConnectionTCPAsio>(mContext, std::move(socket), mListener);
                mActiveConnections.push(std::move(newConn));
                mActiveConnections.back()->ConnectToClient(condition);
                Logger::Info("[Server] NetConnectionTCP Approved.");
            }

            PostHandleAccept(condition);
        }

    private:
        IOContextASIO mContext;
        asio::ip::tcp::acceptor mAcceptor;
        asio::steady_timer mAcceptorTimer;
        bool mIsStarted = false;

        DynamicArray<SharedPtr<ConnectionTCPAsio>> mActiveConnections;
        EventListener<(size_t)NetEvent::COUNT> mListener;
    };
}
}
#endif