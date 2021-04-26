#include "renderPass.h"
#include "renderGraph.h"
#include "renderPassImpl.h"
#include "core\memory\memory.h"
#include "core\container\staticArray.h"

namespace Cjing3D
{
	RenderPass::RenderPass()
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

	void RenderPass::AddRTV(const RenderGraphResource& res, const RenderGraphAttachment& attachment)
	{
		mImpl->AddRTV(res, attachment);
	}

	void RenderPass::SetDSV(const RenderGraphResource& res, const RenderGraphAttachment& attachment)
	{
		mImpl->SetDSV(res, attachment);
	}

	void RenderPass::SetIndex(U32 index)
	{
		mImpl->mIndex = index;
	}

	void RenderPass::AddQueueFlag(RenderGraphQueueFlag queueFlag)
	{
		mImpl->mQueueFlags |= queueFlag;
	}

	Span<const RenderGraphResource> RenderPass::GetInputs() const
	{
		return Span<const RenderGraphResource>(mImpl->mInputs.data(), mImpl->mInputCount);
	}

	Span<const RenderGraphResource> RenderPass::GetOutputs() const
	{
		return Span<const RenderGraphResource>(mImpl->mOutputs.data(), mImpl->mOutputCount);
	}

	Span<RenderGraphResource> RenderPass::GetInputs()
	{
		return Span<RenderGraphResource>(mImpl->mInputs.data(), mImpl->mInputCount);
	}

	Span<RenderGraphResource> RenderPass::GetOutputs()
	{
		return Span<RenderGraphResource>(mImpl->mOutputs.data(), mImpl->mOutputCount);
	}

	U32 RenderPass::GetIndex() const
	{
		return mImpl->mIndex;
	}

	RenderGraphQueueFlags RenderPass::GetQueueFlags() const
	{
		return mImpl->mQueueFlags;
	}
}