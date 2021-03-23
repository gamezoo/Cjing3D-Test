#pragma once

#include "network.h"

namespace Cjing3D
{
namespace Network
{
    class Communication
    {
    public:
        Communication();
        virtual ~Communication();

        ErrorCode Send(const MessagePtr& msg);
        MessagePtr Receive();
    };
}
}