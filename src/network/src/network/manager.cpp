#include "manager.h"

namespace Cjing3D
{
namespace Network
{
    class ManagerImpl
    {

    };
    ManagerImpl* gImpl = nullptr;

    void Manager::Initialize()
    {
        gImpl = CJING_NEW(ManagerImpl);
    }

    void Manager::Uninitialize()
    {
        CJING_SAFE_DELETE(gImpl);
    }

    bool Manager::IsInitialized()
    {
        return gImpl != nullptr;
    }
}
}