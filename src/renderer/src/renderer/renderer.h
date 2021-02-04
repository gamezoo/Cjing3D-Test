#pragma once

#include "core\common\common.h"
#include "definitions.h"
#include "gpu\gpu.h"
#include "renderer\shader.h"
#include "renderer\cullingSystem.h"

namespace Cjing3D
{
	class Engine;
	class RenderScene;
	class Universe;

	namespace Renderer
	{
		void Initialize(GPU::GPUSetupParams params);
		bool IsInitialized();
		void Uninitialize();
		void InitRenderScene(Engine& engine, Universe& universe);
		void Update(CullResult& visibility, FrameCB& frameCB, F32 deltaTime);
		void PresentBegin(GPU::CommandList& cmd);
		void PresentEnd();
		void EndFrame();

		GPU::ResHandle GetConstantBuffer(CBTYPE type);
		ShaderRef GetShader(SHADERTYPE type);
		ShaderRef LoadShader(const char* path, bool waitFor = false);
		RenderScene* GetRenderScene();

		void AddStaticSampler(const GPU::ResHandle& handle, I32 slot);
		void UpdateVisibility(CullResult& visibility, Viewport& viewport, I32 cullingFlag);
		void UpdateCameraCB(const Viewport& viewport, CameraCB& cameraCB);
	}
}