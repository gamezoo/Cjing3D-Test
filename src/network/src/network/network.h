#pragma once

#include "definitions.h"

namespace Cjing3D
{
namespace Network
{
    template<typename DerivedT>
    class NetConnection : public ObjectCRTP<DerivedT>
    {
    public:
        NetConnection() {}
        ~NetConnection() {}

        void Disconnect()
        {
            this->Derived().Disconnect();
        }
      
        bool IsConnected()const
        {
            return this->Derived().IsConnected();
        }

        ConnectionStatus GetStatus()const {
            return this->Derived().GetStatus();
        }
    };

    template<typename DerivedT>
    class ClientInterface : public ObjectCRTP<DerivedT>
    {
    public:
        ClientInterface() {}
        ~ClientInterface() {}

        bool Connect(const String& address, I32 port) 
        {
            return this->Derived().Connect(address, port);
        }

        void Disconnect()
        {
            return this->Derived().Disconnect();
        }

        bool IsConnected()const 
        {
            return this->Derived().IsConnected();
        }

    public:
        template<typename F, typename... T>
        void BindReceive(F&& func, T&&...obj)
        {
            this->Derived().BindReceive(std::forward<F>(func), std::forward<T>(obj)...);
        }
    };


    template<typename DerivedT>
    class ServerInterface : public ObjectCRTP<DerivedT>
    {
    public:
        ServerInterface() {}
        ~ServerInterface() {}

        bool Start(const String& address, I32 port) 
        {
            this->Derived().Start(address, port);
        }

        void Update() 
        {
            this->Derived().Update();
        }

        void Stop() 
        {
            this->Derived().Stop();
        }

        bool IsStarted()const 
        {
            return this->Derived().IsStarted();
        }

    public:
        template<typename F, typename... T>
        void BindReceive(F&& func, T&&...obj)
        {
            this->Derived().BindReceive(std::forward<F>(func), std::forward<T>(obj)...);
        }
    };
}
}