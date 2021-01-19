#pragma once

#include "renderGraph.h"
#include "gpu\gpu.h"

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
		RenderGraphResource mDSV;
		RenderGraphFrameAttachment mDSVAttachment;

		// frameBindingSet
		GPU::ResHandle mFrameBindingSet;
		GPU::FrameBindingSetDesc mFrameBindingSetDesc;

	public:
		~RenderPassImpl() 
		{
			if (mFrameBindingSet != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(mFrameBindingSet);
			}
		}

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
			Debug::CheckAssertion(mRTVCount < GPU::MAX_BOUND_RTVS);
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
}