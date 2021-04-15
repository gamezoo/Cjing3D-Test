#include "renderPass.h"
#include "renderGraph.h"
#include "renderPassImpl.h"
#include "core\memory\memory.h"
#include "core\container\staticArray.h"

namespace Cjing3D
{
	RenderPass::RenderPass(RenderGraphResBuilder& builder)
	{
		mImpl = CJING_NEW(RenderPassImpl);
	}

	RenderPass::~RenderPass()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void RenderPass::AddInput(const RenderGraphResource& res)
	{
		mImpl->AddInput(res);
	}

	void RenderPass::AddOutput(const RenderGraphResource& res)
	{
		mImpl->AddOutput(res);
	}

	void RenderPass::AddRTV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment)
	{
		mImpl->AddRTV(res, attachment);
	}

	void RenderPass::SetDSV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment)
	{
		mImpl->SetDSV(res, attachment);
	}

	Span<const RenderGraphResource> RenderPass::GetInputs() const
	{
		return Span<const RenderGraphResource>(mImpl->mInputs.data(), mImpl->mInputCount);
	}

	Span<const RenderGraphResource> RenderPass::GetOutputs() const
	{
		return Span<const RenderGraphResource>(mImpl->mOutputs.data(), mImpl->mOutputCount);
	}

	void PresentRenderPass::Execute(RenderGraphResources& resources, GPU::CommandList& cmd)
	{
		// do nothing...
	}
}