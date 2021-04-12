#pragma once

#include "definitions.h"
#include "shaderInterop.h"
#include "core\common\common.h"
#include "gpu\gpu.h"
#include "renderer\shader.h"
#include "renderer\model.h"
#include "math\intersectable.h"

namespace Cjing3D
{
	class Engine;
	class RenderScene;
	class Universe;
	class RenderGraphResources;

	namespace Renderer
	{
		void Initialize(GPU::GPUSetupParams params, bool loadShaders = true);
		bool IsInitialized();
		void Uninitialize();
		void InitRenderScene(Engine& engine, Universe& universe);
		void Update(CullingResult& visibility, FrameCB& frameCB, F32 deltaTime);
		void PresentBegin(GPU::CommandList& cmd);
		void PresentEnd();
		void EndFrame();

		// drawing methods
		void DrawScene(RENDERPASS renderPass, RENDERTYPE renderType, const CullingResult& cullResult, RenderGraphResources& resources, GPU::CommandList& cmd);

		GPU::ResHandle GetConstantBuffer(CBTYPE type);
		ShaderRef GetShader(SHADERTYPE type);
		ShaderRef LoadShader(const char* path, bool waitFor = false);
		void LoadAllShaders();
		RenderScene* GetRenderScene();

		void AddStaticSampler(const GPU::ResHandle& handle, I32 slot);
		void UpdateViewCulling(CullingResult& cullingResult, Viewport& viewport, I32 cullingFlag);
		void UpdateCameraCB(const Viewport& viewport, CameraCB& cameraCB);
	}
}