#pragma once

#include "definitions.h"
#include "shaderInterop.h"
#include "core\common\common.h"
#include "gpu\gpu.h"
#include "renderer\shader.h"
#include "renderer\model.h"
#include "renderer\renderGraph\resource.h"
#include "math\intersectable.h"

namespace Cjing3D
{
	class Engine;
	class RenderScene;
	class Universe;
	class RenderGraph;
	class RenderGraphResBuilder;
	class RenderGraphResources;
	class GameWindow;

	namespace Renderer
	{
		void Initialize(GPU::GPUSetupParams params, bool loadShaders = true);
		bool IsInitialized();
		void Uninitialize();
		void InitRenderScene(Engine& engine, Universe& universe);
		void UpdatePerFrameData(Visibility& visibility, FrameCB& frameCB, F32 deltaTime, const U32x2& resolution);
		void EndFrame();
		void AddStaticSampler(const GPU::ResHandle& handle, I32 slot);
		void UpdateViewCulling(Visibility& cullingResult, Viewport& viewport, I32 cullingFlag);
		void UpdateCameraCB(const Viewport& viewport, RenderGraphResources& resources, GPU::CommandList& cmd);

		// render graph path
		void SetupRenderData(RenderGraph& renderGraph, const FrameCB& frameCB, const Visibility& visibility, Viewport& viewport);
		void DrawShadowMaps(RenderGraph& renderGraph, const Visibility& visibility);
		void DrawScene(RENDERPASS renderPass, RENDERTYPE renderType, const Visibility& cullResult, RenderGraphResources& resources, GPU::CommandList& cmd);

		// status
		RenderGraphResource GetConstantBuffer(CBTYPE type);
		ShaderRef GetShader(SHADERTYPE type);
		ShaderRef LoadShader(const char* path, bool waitFor = false);
		void LoadAllShaders();
		RenderScene* GetRenderScene();
		U32x2 GetInternalResolution();
		void SetWindow(const GameWindow& gameWindow);
	}
}