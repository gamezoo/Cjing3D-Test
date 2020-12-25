#include "device.h"

namespace Cjing3D {
namespace GPU
{
    GraphicsDevice::GraphicsDevice(GraphicsDeviceType type) :
        mDeviceType(type)
    {
    }

    GraphicsDevice::~GraphicsDevice()
    {
    }
}
}