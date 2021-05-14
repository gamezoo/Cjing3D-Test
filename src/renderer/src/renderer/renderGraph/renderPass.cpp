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

	void RenderPass::AddInput(const ResourceNode* res)
	{
		mImpl->AddInput(res);
	}

	void RenderPass::AddOutput(const ResourceNode* res)
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

	Span<const ResourceNode*> RenderPass::GetInputs() const
	{
		return Span<const ResourceNode*>(mImpl->mInputs.data(), mImpl->mInputCount);
	}

	Span<const ResourceNode*> RenderPass::GetOutputs() const
	{
		return Span<const ResourceNode*>(mImpl->mOutputs.data(), mImpl->mOutputCount);
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