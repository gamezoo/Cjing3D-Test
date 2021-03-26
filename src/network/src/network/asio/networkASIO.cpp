#include "networkASIO.h"
#include "core\string\stringUtils.h"
#include "core\helper\timer.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    namespace 
    {
        thread_local static asio::error_code errorCodeIgnore;

    }

    //////////////////////////////////////////////////////////////////////////
    // IOContext
    //////////////////////////////////////////////////////////////////////////

    IOContextASIO::IOContextASIO() :
        mStrand(mContext)
    {
    }

    IOContextASIO::~IOContextASIO()
    {
        Stop();
    }

    bool IOContextASIO::Start()
    {
        if (!mIsStoped) {
            return false;
        }

        if (mWork || mThreadContext.IsValid()) {
            return false;
        }

        mContext.restart();

        // create work guard
        // Issue a task to the asio context - This is important
        // as it will prime the context with "work", and stop it
        // from exiting immediately. Since this is a server, we 
        // want it primed ready to handle clients trying to
        // connect.
        mWork = CJING_NEW(asio::executor_work_guard<asio::io_context::executor_type>)(mContext.get_executor());
 
        mThreadContext = Concurrency::Thread([](void* data) {
            asio::io_context* context = reinterpret_cast<asio::io_context*>(data);
            context->run();
            return 0;
            }, &mContext, 4096, "IOContextThread");
        mIsStoped = false;
    }

    void IOContextASIO::Stop()
    {
        if (mIsStoped) {
            return;
        }

        if (!mWork && !mThreadContext.IsValid()) {
            return;
        }

        // waiting for all works to complete. 
        Wait();
        // clear work guard
        mWork->reset();
     
        if (mThreadContext.IsValid()) {
            mThreadContext.Join();
        }

        CJING_DELETE(mWork);
        mIsStoped = true;
    }

    void IOContextASIO::Wait()
    {
        // reset work guard
        if (mWork != nullptr) {
            mWork->reset();
        }

        // wait until the io_context is stopped
        auto time = Timer::GetAbsoluteTime();
        while (!mContext.stopped()) {
            F32 sleepTime = (F32)std::max(std::min(Timer::GetAbsoluteTime() - time, 0.01), 0.001);
            Logger::Info("Sleep:%f", sleepTime);
            Concurrency::Sleep(sleepTime);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Connectoin
    //////////////////////////////////////////////////////////////////////////

    ConnectionAsio::ConnectionAsio(IOContextASIO& context, asio::ip::tcp::socket socket, EventListener<(size_t)NetEvent::COUNT>& listener) :
        mContext(context),
        mSocket(std::move(socket)),
        mStatus(ConnectionStatus::Connecting),
        mListener(listener)
    {
        mRecvBuffer.Reserve(DEFAULT_TCP_BUFFER_SIZE);
    }

    ConnectionAsio::~ConnectionAsio()
    {
    }

    void ConnectionAsio::Disconnect()
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

    void ConnectionAsio::ConnectToClient()
    {
        try
        {
            ConnectionStatus expected = ConnectionStatus::Connecting;
            if (Concurrency::AtomicCmpExchange((I32*)(&mStatus),
                (I32)ConnectionStatus::Connected, (I32)expected) != (I32)ConnectionStatus::Connecting) {
                asio::detail::throw_error(asio::error::already_started);
            }

            // post to read message from client
            PostAsyncRead();
        }
        catch (std::exception& e)
        {
            Logger::Warning("[Server] Failed to connect to client:%s", e.what());
            Disconnect();
        }
    }

    void ConnectionAsio::ConnectToServer(const asio::ip::tcp::resolver::results_type& endPoints)
    {
        asio::async_connect(mSocket, endPoints,
            [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                if (!ec) {  
                    HandleConnect(ec, endpoint);
                }
                else
                {
                    Logger::Warning("[Client] Failed to connect to %s:%d", endpoint.address().to_string().c_str(), endpoint.port());
                    Disconnect();
                }
            });
    }

    void ConnectionAsio::PostAsyncRead()
    {
        if (!IsConnected()) {
            return;
        }
        try
        {
            void* buf = (void*)mRecvBuffer.OffsetData();
            U32 size = mRecvBuffer.GetCapacity();
            asio::async_read(mSocket, asio::buffer(buf, size), asio::bind_executor(mContext.GetStrand(), 
                [this](const asio::error_code& ec, std::size_t bytesReceived) {
                    HandleRead(ec, bytesReceived);
                }));

        }
        catch (std::exception& e)
        {
            Logger::Warning("[Server] NetConnection:%s", e.what());
            Disconnect();
        }
    }

    void ConnectionAsio::PostDisconnect()
    {
        asio::post(mContext.GetStrand(), [this]() 
        {
            ConnectionStatus expected = ConnectionStatus::Closing;
            Concurrency::AtomicCmpExchange((I32*)(&mStatus), (I32)ConnectionStatus::Closed, (I32)expected);

            // close socket
            if (mSocket.is_open()) {
                Logger::Info("[%s] NetConnection disconnected.", mSocket.remote_endpoint().address().to_string().c_str());
            }
            else {
                Logger::Info("NetConnection disconnected.");
            }

            asio::post(mContext.GetStrand(), [this]() {
                mSocket.shutdown(asio::socket_base::shutdown_both, errorCodeIgnore);
                mSocket.close();
            });
        });
    }

    void ConnectionAsio::HandleConnect(asio::error_code ec, asio::ip::tcp::endpoint endpoint)
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
        }
        catch (std::exception& e)
        {
            Logger::Warning("[Client] Failed to connect to %s:%d",
                endpoint.address().to_string().c_str(), endpoint.port());
            Disconnect();
        }
    }

    void ConnectionAsio::HandleRead(const asio::error_code& ec, std::size_t bytesReceived)
    {   
        if (ec)
        {
            // try read if error is 'would_block' or 'try_again', otherwise close connection
            if (ec.value() == asio::error::would_block ||
                ec.value() == asio::error::try_again) {
                PostAsyncRead();
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
                mListener.FireEvent(NetEvent::RECEIVE, shared_from_this(), Span(buf, bytesReceived));
            }

            mRecvBuffer.ConsumeSize(bytesReceived, true);
            PostAsyncRead();
        }
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
            mConnection = CJING_MAKE_UNIQUE<ConnectionAsio>(mContext, asio::ip::tcp::socket(mContext.GetContext()), mListener);
        
            // connect to server
            mConnection->ConnectToServer(endPoints);

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

    void ClientInterfaceASIO::Disconnect()
    {
        if (mConnection)
        {
            mConnection->Disconnect();
            mContext.Stop();
            mConnection.Reset();
        }
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
        mAcceptor(mContext.GetContext(), asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        mAcceptorTimer(mContext.GetContext())
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
            // Launch the asio context in its own thread
            mContext.Start();

            // set flag
            mIsStarted = true;

            // Wait for client connections
            PostHandleAccept();
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
        // close acceptor
        mAcceptorTimer.cancel();
        asio::post(mContext.GetStrand(), [this]() {
            mAcceptor.close(errorCodeIgnore);
        });    
       
        // stop io context
        mContext.Stop();

        mIsStarted = false;
        Logger::Info("[Server] Server stopped.");
    }

    bool ServerInterfaceASIO::IsStarted() const
    {
        return mIsStarted && mAcceptor.is_open();
    }

    void ServerInterfaceASIO::PostHandleAccept()
    {
        if (!IsStarted()) {
            return;
        }
        try
        {
            mAcceptor.async_accept(asio::bind_executor(mContext.GetStrand(), [this](std::error_code ec, asio::ip::tcp::socket socket) {
                HandleAccept(std::move(socket), ec);
            }));
        }
        catch (std::exception& e)
        {
            Logger::Warning("[Server] Acceptor:%s", e.what());

            // there some exception, do WaitForClientConnection in a seconds
            mAcceptorTimer.expires_after(std::chrono::seconds(1));
            mAcceptorTimer.async_wait(asio::bind_executor(mContext.GetStrand(), 
                [this](const asio::error_code& ec) {
                    if (ec) {
                        return;
                    }
                    asio::post(mContext.GetStrand(), [this]() {
                        PostHandleAccept();
                     });
                }));
        }
    }
    void ServerInterfaceASIO::HandleAccept(asio::ip::tcp::socket&& socket, std::error_code ec)
    {
        // acceptor is closed, just return
        if (ec == asio::error::operation_aborted) {
            return;
        }
        if (ec) {
            Logger::Error("[Server] Error connection:%s", ec.message());
        }
        else
        {
            Logger::Info("[Server] New NetConnection:%s:%d", socket.remote_endpoint().address().to_string().c_str(), socket.remote_endpoint().port());
            SharedPtr<ConnectionAsio> newConn = CJING_MAKE_SHARED<ConnectionAsio>(mContext, std::move(socket), mListener);
            if (OnClientConnect(newConn))
            {
                mActiveConnections.push(std::move(newConn));
                mActiveConnections.back()->ConnectToClient();
                Logger::Info("[Server] NetConnection Approved.");
            }
            else {
                Logger::Warning("[-----] NetConnection Denied.");
            }
        }

        PostHandleAccept();
    }
}
}
#endif