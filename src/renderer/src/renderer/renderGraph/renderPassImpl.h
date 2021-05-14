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
		U32 mIndex = ~0;
		RenderGraphQueueFlags mQueueFlags = 0;

		I32 mInputCount = 0;
		StaticArray<const ResourceNode*, MAX_RES_INPUTS> mInputs;
		I32 mOutputCount = 0;
		StaticArray<const ResourceNode*, MAX_RES_INPUTS> mOutputs;

		// rtv
		I32 mRTVCount = 0;
		StaticArray<RenderGraphResource, GPU::MAX_BOUND_RTVS> mRTVs;
		StaticArray<RenderGraphAttachment, GPU::MAX_BOUND_RTVS> mRTVAttachments;

		// dsv
		RenderGraphResource mDSV;
		RenderGraphAttachment mDSVAttachment;

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

		void AddInput(const ResourceNode* node)
		{
			Debug::CheckAssertion(mInputCount < mInputs.size());
			mInputs[mInputCount++] = node;
		}

		void AddOutput(const ResourceNode* node)
		{
			Debug::CheckAssertion(mOutputCount < mOutputs.size());
			mOutputs[mOutputCount++] = node;
		}

		void AddRTV(const RenderGraphResource& res, const RenderGraphAttachment& attachment)
		{
			Debug::CheckAssertion(mRTVCount < GPU::MAX_BOUND_RTVS);
			mRTVs[mRTVCount] = res;
			mRTVAttachments[mRTVCount] = attachment;
			mRTVCount++;
		}

		void SetDSV(const RenderGraphResource& res, const RenderGraphAttachment& attachment)
		{
			mDSV = res;
			mDSVAttachment = attachment;
		}
	};
}