#include "renderPass.h"
#include "renderGraph.h"
#include "core\memory\memory.h"
#include "core\container\staticArray.h"

namespace Cjing3D
{
	const I32 MAX_RES_INPUTS = 16;
	const I32 MAX_RES_OUTPUTS = 16;

	class RenderPassImpl
	{
	public:
		I32 mInputCount = 0;
		StaticArray<RenderGraphResource, MAX_RES_INPUTS> mInputs;
		I32 mOutputCount = 0;
		StaticArray<RenderGraphResource, MAX_RES_INPUTS> mOutputs;

		// rtv
		I32 mRTVCount = 0;
		StaticArray<RenderGraphResource, GPU::MAX_BOUND_RTVS> mRTVs;
		StaticArray<RenderGraphFrameAttachment, GPU::MAX_BOUND_RTVS> mRTVAttachments;

		// dsv
		I32 mDSVCount = 0;
		RenderGraphResource mDSV;
		RenderGraphFrameAttachment mDSVAttachment;

		void AddInput(const RenderGraphResource& res)
		{
			Debug::CheckAssertion(mInputCount < mInputs.size());
			mInputs[mInputCount++] = res;
		}

		void AddOutput(const RenderGraphResource& res)
		{
			Debug::CheckAssertion(mOutputCount < mOutputs.size());
			mOutputs[mOutputCount++] = res;
		}

		void AddRTV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment)
		{
			Debug::CheckAssertion(mDSVCount < GPU::MAX_BOUND_RTVS);
			mRTVs[mRTVCount] = res;
			mRTVAttachments[mRTVCount] = attachment;
			mRTVCount++;
		}

		void SetDSV(const RenderGraphResource& res, const RenderGraphFrameAttachment& attachment)
		{
			mDSV = res;
			mDSVAttachment = attachment;
		}
	};

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
}