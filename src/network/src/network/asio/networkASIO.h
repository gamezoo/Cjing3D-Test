#pragma once
#include "network\network.h"
#include "core\container\dynamicArray.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    //////////////////////////////////////////////////////////////////////////
    // Connectoin
    //////////////////////////////////////////////////////////////////////////
    class ConnectionAsio : public Connection
    {
    public:
        ConnectionAsio(asio::io_context& context, asio::ip::tcp::socket socket);
        virtual ~ConnectionAsio();

        void Update();
        void Disconnect()override;
        void Send()override;
        void Receive()override;
        bool IsConnected()const override;

        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endPoints);

        ConnectionStatus GetStatus()const override {
            return mStatus;
        }
        bool IsDirty()const {
            return mIsDirty;
        }

    private:
        asio::io_context& mContext;
        asio::ip::tcp::socket mSocket;
        ConnectionStatus mStatus;
        bool mIsDirty = false;
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
        asio::io_context mContext;
        Concurrency::Thread mThreadContext;
        UniquePtr<ConnectionAsio> mConnection;
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

    private:
        void WaitForClientConnection();

    private:
        asio::io_context mContext;
        Concurrency::Thread mThreadContext;
        asio::ip::tcp::acceptor mAcceptor;
        asio::ip::tcp::endpoint mEndPoint;

        DynamicArray<SharedPtr<ConnectionAsio>> mActiveConnections;
    };
}
}
#endif