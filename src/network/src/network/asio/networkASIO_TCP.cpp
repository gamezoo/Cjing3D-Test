#include "networkASIO_TCP.h"
#include "core\string\stringUtils.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
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
        Debug::CheckAssertion(port > 1024 && port < 65536);
        return ConnectImpl(address, port, ConditionWrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
    }

    void ClientInterfaceASIO::Disconnect()
    {
        if (mConnection)
        {
            mConnection->Disconnect();
            mContext.Stop();
            mConnection.reset();
        }
    }

    bool ClientInterfaceASIO::IsConnected() const
    {
        return mConnection ? mConnection->IsConnected() : false;
    }

    //////////////////////////////////////////////////////////////////////////
    // ServerInterface
    //////////////////////////////////////////////////////////////////////////

    ServerInterfaceASIO::ServerInterfaceASIO() :
        mAcceptor(mContext.GetContext()),
        mAcceptorTimer(mContext.GetContext())
    {
    }

    ServerInterfaceASIO::~ServerInterfaceASIO()
    {
        Stop();
    }

    bool ServerInterfaceASIO::Start(const String& address, I32 port)
    {
        Debug::CheckAssertion(port > 1024 && port < 65536);
        return StartImpl(address, port, ConditionWrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
    }

    void ServerInterfaceASIO::Update()
    {
        if (!mActiveConnections.empty())
        {
            mActiveConnections.erase(std::remove_if(mActiveConnections.begin(), mActiveConnections.end(),
                [](const SharedPtr<ConnectionTCPAsio>& conn) {
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
}
}
#endif