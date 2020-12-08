#pragma once

#include "gpu\device.h"
#include "core\common\common.h"

namespace Cjing3D
{
namespace Renderer
{
	void Initialize();
	bool IsInitialized();
	void Uninitialize();
	void SetDevice(SharedPtr<GraphicsDevice> device);
	GraphicsDevice* GetDevice();
}
}