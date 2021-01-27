#pragma once

#include "core\common\common.h"
#include "definitions.h"
#include "gpu\gpu.h"
#include "renderer\shader.h"
#include "renderer\cullingSystem.h"

namespace Cjing3D
{
	class Universe;
	class Engine;

	namespace Renderer
	{
		void Initialize(GPU::GPUSetupParams params);
		bool IsInitialized();
		void Uninitialize();
		void InitRenderScene(Engine& engine, Universe& universe);
		void Update(CullResult& visibility, F32 deltaTime);
		void PresentBegin(GPU::CommandList& cmd);
		void PresentEnd();
		void EndFrame();

		ShaderRef GetShader(SHADERTYPE type);
		ShaderRef LoadShader(const char* path, bool waitFor = false);

		void UpdateVisibility(CullResult& visibility, Viewport& viewport, I32 cullingFlag);
	}
}