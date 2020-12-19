#include "commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D {
namespace GPU {

    CommandList::CommandList()
    {
    }

    CommandList::~CommandList()
    {
        if (mHandle != ResHandle::INVALID_HANDLE) {
            GPU::DestroyResource(mHandle);
        }
    }

    void CommandList::Reset()
    {
    }
}
}