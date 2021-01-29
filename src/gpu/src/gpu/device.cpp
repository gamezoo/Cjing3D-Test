#include "device.h"
#include "core\platform\platform.h"

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

    F32x2 GraphicsDevice::GetScreenSize() const
    {
        return F32x2((F32)GetResolutionWidth(), (F32)GetResolutionHeight()) / (F32)Platform::GetDPI();
    }
}
}