#pragma once

#include "renderGraphPath.h"
#include "renderer\renderPipeline\renderPipeline2D.h"

namespace Cjing3D
{
	class RenderGraphPath2D : public RenderGraphPath
	{
	public:
		RenderGraphPath2D();
		virtual ~RenderGraphPath2D();

		void Start()override;
		void Stop()override;
		void Update(F32 dt)override;
		void UpdatePipelines(RenderGraph& renderGraph)override;
		void Compose(GPU::CommandList& cmd)override;

		void ResizeBuffers();

		static const char* RT_MAIN_NAME;

	protected:
		ScopedConnection mResolutionChangedHandle;
		RenderPipeline2D mRenderPipeline2D;

		Texture mRtMain;
	};
}