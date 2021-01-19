#pragma once

#include "renderPath.h"
#include "renderer\renderGraph.h"
#include "renderer\renderPipeline\mainRenderPipeline.h"
#include "renderer\texture.h"
#include "core\event\eventSystem.h"

namespace Cjing3D
{
	class RenderGraphPath : public RenderPath
	{
	public:
		RenderGraphPath();
		virtual ~RenderGraphPath();

		void Start()override;
		void Stop()override;
		void Update(F32 dt)override;
		void FixedUpdate()override;
		void Render()override;
		void Compose()override;

		void Clear();
		void ResizeBuffers();

	private:
		RenderGraph mRenderGraph;
		MainRenderPipeline mMainPipeline;
		ScopedConnection mResolutionChangedHandle;

		Texture mRTMain;
	};
}