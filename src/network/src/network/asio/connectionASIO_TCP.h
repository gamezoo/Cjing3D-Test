#pragma once

#include "network\network.h"
#include "network\asio\networkASIO_impl.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    // Connection(ASIO impl)
    // Connection持有Disconnect和Connect(ToClient\ToServer)以及对应的Recv\Send
    // 
    // Clinet: Clinet仅持有一个Connection，执行ConnectToServer
    // Server: Server根据连接的Client,执行ConnectToClient

    class ConnectionTCPAsio :
        public NetConnectionTCP<ConnectionTCPAsio>,
        public EventQueue,
        public Sender<ConnectionTCPAsio>,
        public DataPersistence<ConnectionTCPAsio>
    {
    public:
        ConnectionTCPAsio(IOContextASIO& context, asio::ip::tcp::socket socket, EventListener<(size_t)NetEvent::COUNT>& listener) :
            EventQueue(context),
            mContext(context),
            mSocket(std::move(socket)),
            mStatus(ConnectionStatus::Connecting),
            mListener(listener)
        {
            mRecvBuffer.Reserve(DEFAULT_TCP_BUFFER_SIZE);
        }
        ~ConnectionTCPAsio() {}

    public:
        // devried function
        void Disconnect()
        {
            ConnectionStatus expected = ConnectionStatus::Connecting;
            if (Concurrency::AtomicCmpExchange((I32*)(&mStatus),
                (I32)ConnectionStatus::Closing, (I32)expected) == (I32)ConnectionStatus::Connecting) {
                PostDisconnect();
            }

            expected = ConnectionStatus::Connected;
            if (Concurrency::AtomicCmpExchange((I32*)(&mStatus),
                (I32)ConnectionStatus::Closing, (I32)expected) == (I32)ConnectionStatus::Connected) {
                PostDisconnect();
            }
        }

        bool IsConnected()const {
            return mSocket.is_open() && mStatus == ConnectionStatus::Connected;
        }

        ConnectionStatus GetStatus()const {
            return mStatus;
        }

        // handleSend called by Sender
        template<typename BufferT>
        bool HandleSend(BufferT& buffer) {
            return HandleWrite(asio::buffer(buffer));
        }

    public:

        template<typename TransferCondition>
        void ConnectToClient(ConditionWrap<TransferCondition> condition)
        {
            try
            {
                ConnectionStatus expected = ConnectionStatus::Connecting;
                if (Concurrency::AtomicCmpExchange((I32*)(&mStatus),
                    (I32)ConnectionStatus::Connected, (I32)expected) != (I32)ConnectionStatus::Connecting) {
                    asio::detail::throw_error(asio::error::already_started);
                }

                // post to read message from client
                asio::post(mContext.GetStrand(), [this, condition]() {
                    PostAsyncRead(std::move(condition));
                });
            }
            catch (std::exception& e)
            {
                Logger::Warning("[Server] Failed to connect to client:%s", e.what());
                Disconnect();
            }
        }

        template<typename TransferCondition>
        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endPoints, ConditionWrap<TransferCondition> condition)
        {
            asio::async_connect(mSocket, endPoints,
                [this, condition](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                    if (!ec) {
                        HandleConnect(ec, endpoint, std::move(condition));
                    }
                    else
                    {
                        Logger::Warning("[Client] Failed to connect to %s:%d", endpoint.address().to_string().c_str(), endpoint.port());
                        Disconnect();
                    }
                });
        }

        template<typename TransferCondition>
        void PostAsyncRead(ConditionWrap<TransferCondition> condition)
        {
            if (!IsConnected()) {
                return;
            }
            try
            {
                if constexpr (
                    std::is_same_v<TransferCondition, asio::detail::transfer_all_t> ||
                    std::is_same_v<TransferCondition, asio::detail::transfer_at_least_t> ||
                    std::is_same_v<TransferCondition, asio::detail::transfer_exactly_t>)
                {
                    void* buf = (void*)mRecvBuffer.OffsetData();
                    U32 size = mRecvBuffer.GetCapacity();
                    asio::async_read(mSocket, asio::buffer(buf, size), condition(), asio::bind_executor(mContext.GetStrand(),
                        [this, condition](const asio::error_code& ec, std::size_t bytesReceived) {
                            HandleRead(ec, bytesReceived, std::move(condition));
                        }));
                }
                else {
                    Debug::ThrowIfFailed(false, "Unsupported transfer condition.");
                }
            }
            catch (std::exception& e)
            {
                Logger::Warning("[Server] NetConnectionTCP:%s", e.what());
                Disconnect();
            }
        }

    private:
        void PostDisconnect()
        {
            asio::post(mContext.GetStrand(), [this]()
            {
                ConnectionStatus expected = ConnectionStatus::Closing;
                Concurrency::AtomicCmpExchange((I32*)(&mStatus), (I32)ConnectionStatus::Closed, (I32)expected);

                // close socket
                if (mSocket.is_open()) {
                    Logger::Info("[%s] NetConnectionTCP disconnected.", mSocket.remote_endpoint().address().to_string().c_str());
                }
                else {
                    Logger::Info("NetConnectionTCP disconnected.");
                }

                asio::post(mContext.GetStrand(), [this]() {
                    mSocket.shutdown(asio::socket_base::shutdown_both, errorCodeIgnore);
                    mSocket.close();
                    });
            });
        }

        template<typename TransferCondition>
        void HandleConnect(asio::error_code ec, asio::ip::tcp::endpoint endpoint, ConditionWrap<TransferCondition> condition)
        {
            try
            {
                // set current status is Connected, if it fails, reconnect to server
                ConnectionStatus expected = ConnectionStatus::Connecting;
                if (Concurrency::AtomicCmpExchange((I32*)(&mStatus),
                    (I32)ConnectionStatus::Connected, (I32)expected) != (I32)ConnectionStatus::Connecting) {
                    ec = asio::error::operation_aborted;
                }

                // fire connect event
                if (mFireConnectFlag.TestAndSet()) {
                    mListener.FireEvent(NetEvent::CONNECT, ec);
                }

                asio::detail::throw_error(ec);
                Logger::Info("[Client] Connected to %s:%d", endpoint.address().to_string().c_str(), endpoint.port());

                // post to read message from client
                asio::post(mContext.GetStrand(), [this, condition]() {
                    PostAsyncRead(std::move(condition));
                });
            }
            catch (std::exception& e)
            {
                Logger::Warning("[Client] Failed to connect to %s:%d:%s",
                    endpoint.address().to_string().c_str(), endpoint.port(), e.what());
                Disconnect();
            }
        }

        template<typename TransferCondition>
        void HandleRead(const asio::error_code& ec, std::size_t bytesReceived, ConditionWrap<TransferCondition> condition)
        {
            if (ec)
            {
                // try read if error is 'would_block' or 'try_again', otherwise close connection
                if (ec.value() == asio::error::would_block ||
                    ec.value() == asio::error::try_again) {
                    PostAsyncRead(std::move(condition));
                }
                else {
                    Disconnect();
                }
            }
            else
            {
                // notify receive event
                if (bytesReceived > 0)
                {
                    char* buf = (char*)mRecvBuffer.OffsetData();
                    SharedPtr<ConnectionTCPAsio> ptr = this->shared_from_this();
                    mListener.FireEvent(NetEvent::RECEIVE, ptr, Span(buf, bytesReceived));
                }

                mRecvBuffer.ConsumeSize(bytesReceived, true);
                PostAsyncRead(std::move(condition));
            }
        }

        bool HandleWrite(asio::mutable_buffer&& buffer)
        {
            asio::async_write(mSocket, buffer, asio::bind_executor(mContext.GetStrand(),
                [this](std::error_code ec, size_t bytesSent) {
                    if (ec) {
                        Disconnect();
                    }

                    // do next Event
                    DoNextEvent();
                }));
            return true;
        }

        bool HandleWrite(asio::const_buffer&& buffer)
        {
            asio::async_write(mSocket, buffer, asio::bind_executor(mContext.GetStrand(),
                [this](std::error_code ec, size_t bytesSent) {
                    if (ec) {
                        Disconnect();
                    }

                    // do next Event
                    DoNextEvent();
                }));
            return true;
        }

    private:
        // common
        IOContextASIO& mContext;
        asio::ip::tcp::socket mSocket;
        MemoryStream mRecvBuffer;
        EventListener<(size_t)NetEvent::COUNT>& mListener;
        volatile ConnectionStatus mStatus;

        // client
        Concurrency::AtomicFlag mFireConnectFlag;
    };
}
}
#endif