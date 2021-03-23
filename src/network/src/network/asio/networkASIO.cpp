#include "networkASIO.h"
#include "core\string\stringUtils.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    //////////////////////////////////////////////////////////////////////////
    // Connectoin
    //////////////////////////////////////////////////////////////////////////

    ConnectionAsio::ConnectionAsio(asio::io_context& context, asio::ip::tcp::socket socket) :
        mContext(context),
        mSocket(std::move(socket)),
        mStatus(ConnectionStatus::Connected)
    {
    }

    ConnectionAsio::~ConnectionAsio()
    {
    }

    void ConnectionAsio::Update()
    {
    }

    void ConnectionAsio::Disconnect()
    {
        if (IsConnected())
        {
            asio::post(mContext, [this]() { 
                mSocket.close(); 
                mStatus = ConnectionStatus::Closed;
                mIsDirty = false;
                Logger::Info("Connection disconnected.");
            });
        }
    }

    void ConnectionAsio::Send()
    {
    }
        
    void ConnectionAsio::Receive()
    {
    }

    bool ConnectionAsio::IsConnected() const
    {
        return mSocket.is_open() && mStatus == ConnectionStatus::Connected;
    }

    void ConnectionAsio::ConnectToServer(const asio::ip::tcp::resolver::results_type& endPoints)
    {
        mStatus = ConnectionStatus::Connecting;

        asio::async_connect(mSocket, endPoints,
            [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                if (!ec)
                {
                    Logger::Info("[Client] Connected to %s:%d", endpoint.address().to_string().c_str(), endpoint.port());
                    mStatus = ConnectionStatus::Connected;
                }
                else
                {
                    Logger::Warning("[Client] Failed to connect to %s:%d", endpoint.address().to_string().c_str(), endpoint.port());
                    mStatus = ConnectionStatus::Closed;
                }
            });
    }

    //////////////////////////////////////////////////////////////////////////
    // ClientInterface
    //////////////////////////////////////////////////////////////////////////

    ClientInterfaceASIO::ClientInterfaceASIO()
    {
    }

    ClientInterfaceASIO::~ClientInterfaceASIO()
    {
        Disconnect();
    }

    bool ClientInterfaceASIO::Connect(const String& address, I32 port)
    {
        try
        {
            // Resolve ip-address into tangiable physical address
            asio::ip::tcp::resolver resolver(mContext);
            StaticString<16> portStr;
            StringUtils::ToString(port, portStr.toSpan());
            asio::ip::tcp::resolver::results_type endPoints = resolver.resolve(address.c_str(), portStr.c_str());

            // create client connection
            mConnection = CJING_MAKE_UNIQUE<ConnectionAsio>(mContext, asio::ip::tcp::socket(mContext));
        
            // connect to server
            mConnection->ConnectToServer(endPoints);

            // Launch the asio context in its own thread
            mThreadContext = Concurrency::Thread([](void* data) {
                asio::io_context* context = reinterpret_cast<asio::io_context*>(data);
                context->run();
                return 0;
                }, &mContext, 1024, "ServerContextThread");
        }
        catch (std::exception& e)
        {
            Logger::Error("[Client] Exception:%s", e.what());
            return false;
        }
        return true;
    }

    void ClientInterfaceASIO::Disconnect()
    {
        if (IsConnected()) {
            mConnection->Disconnect();
        }

        // stop the context and the thread
        mContext.stop();

        if (mThreadContext.IsValid()) {
            mThreadContext.Join();
        }

        mConnection.Reset();
    }

    bool ClientInterfaceASIO::IsConnected() const
    {
        return mConnection ? mConnection->IsConnected() : false;
    }

    //////////////////////////////////////////////////////////////////////////
    // ServerInterface
    //////////////////////////////////////////////////////////////////////////

    ServerInterfaceASIO::ServerInterfaceASIO(I32 port) :
        mEndPoint(asio::ip::tcp::v4(), port),
        mAcceptor(mContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
        Debug::CheckAssertion(port > 1024 && port < 65536);
    }

    ServerInterfaceASIO::~ServerInterfaceASIO()
    {
        Stop();
    }

    bool ServerInterfaceASIO::Start()
    {
        try
        {
            // push a task to the asio context
            WaitForClientConnection();

            // Launch the asio context in its own thread
            mThreadContext = Concurrency::Thread([](void* data) {
                asio::io_context* context = reinterpret_cast<asio::io_context*>(data);
                context->run();
                return 0;
             }, &mContext, 1024, "ServerContextThread");


        }
        catch (std::exception& e)
        {
            Logger::Error("[Server] Exception:%s", e.what());
            return false;
        }

        Logger::Info("[Server] Server started.");
        return true;
    }

    void ServerInterfaceASIO::Update()
    {
        if (!mActiveConnections.empty())
        {
            mActiveConnections.erase(std::remove_if(mActiveConnections.begin(), mActiveConnections.end(),
                [](const SharedPtr<ConnectionAsio>& conn) {
                    return conn->GetStatus() == ConnectionStatus::Closed;
                }),
                mActiveConnections.end());
        }
    }

    void ServerInterfaceASIO::Stop()
    {
        mContext.stop();

        if (mThreadContext.IsValid()) {
            mThreadContext.Join();
        }
        Logger::Info("[Server] Server stopped.");
    }

    void ServerInterfaceASIO::WaitForClientConnection()
    {
        mAcceptor.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket)
            {
                if (ec) {
                    Logger::Error("[Server] Error connection:%s", ec.message());
                }
                else
                {
                    Logger::Info("[Server] New Connection:%s:%d", socket.remote_endpoint().address().to_string().c_str(), socket.remote_endpoint().port());
                    SharedPtr<ConnectionAsio> newConn = CJING_MAKE_SHARED<ConnectionAsio>(mContext, std::move(socket));
                    if (OnClientConnect(newConn))
                    {
                        mActiveConnections.push(std::move(newConn));
                        Logger::Info("[Server] Connection Approved.");
                    }
                    else {
                        Logger::Warning("[-----] Connection Denied.");
                    }
                }

                WaitForClientConnection();
            });
    }
}
}
#endif