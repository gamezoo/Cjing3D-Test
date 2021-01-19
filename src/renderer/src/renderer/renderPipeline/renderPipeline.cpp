#include "renderPipeline.h"

namespace Cjing3D
{
	RenderPipeline::RenderPipeline()
	{
	}

	RenderPipeline::~RenderPipeline()
	{
	}

	void RenderPipeline::SetResource(const char* name, RenderGraphResource res)
	{
		mResources.insert(name, res);
	}

	RenderGraphResource RenderPipeline::GetResource(const char* name)
	{
		auto it = mResources.find(name);
		if (it != nullptr) {
			return *it;
		}
		return RenderGraphResource();
	}

	void RenderPipeline::Clear()
	{
		mResources.clear();
	}
}