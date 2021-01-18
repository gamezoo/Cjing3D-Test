#pragma once

#include "renderPath.h"
#include "renderer\renderGraph.h"

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

	private:
		RenderGraph mGraph;
	};
}