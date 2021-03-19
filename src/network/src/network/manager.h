#pragma once

#include "core\common\common.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
namespace Network
{
    namespace Manager
    {
        void Initialize();
        void Uninitialize();
        bool IsInitialized();
    };
}
}