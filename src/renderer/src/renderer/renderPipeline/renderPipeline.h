#pragma once

#include "renderer\renderGraph.h"
#include "core\container\hashMap.h"

namespace Cjing3D
{
	class RenderPipeline
	{
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void SetResource(const char* name, RenderGraphResource res);
		RenderGraphResource GetResource(const char* name);
		void Clear();

	private:
		HashMap<String, RenderGraphResource> mResources;
	};
}