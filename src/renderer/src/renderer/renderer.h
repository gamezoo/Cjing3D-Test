#pragma once

#include "core\common\common.h"
#include "gpu\gpu.h"
#include "shader.h"

namespace Cjing3D
{
namespace Renderer
{
	void Initialize(GPU::GPUSetupParams params);
	bool IsInitialized();
	void Uninitialize();
	void PresentBegin(GPU::CommandList& cmd);
	void PresentEnd();
	void EndFrame();
}
}