#include "comm.h"

namespace Cjing3D
{
namespace Network
{
    Communication::Communication()
    {
    }

    Communication::~Communication()
    {
    }

    ErrorCode Communication::Send(const MessagePtr& msg)
    {
        return ErrorCode();
    }

    MessagePtr Communication::Receive()
    {
        return MessagePtr();
    }
}
}