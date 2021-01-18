#pragma once

#include "gpu\definitions.h"
#include "gpu\resource.h"

namespace Cjing3D
{
	enum SHADERTYPE
	{
		SHADERTYPE_DEFAULT,
		SHADERTYPE_COUNT
	};

	struct RenderGraphFrameAttachment
	{
		GPU::BindingFrameAttachment::TYPE mType = GPU::BindingFrameAttachment::RENDERTARGET;
		GPU::BindingFrameAttachment::LoadOperation mLoadOperator = GPU::BindingFrameAttachment::LOAD_DEFAULT;

		I32 mSubresourceIndex = -1;

		static RenderGraphFrameAttachment RenderTarget(
			GPU::BindingFrameAttachment::LoadOperation loadOp = GPU::BindingFrameAttachment::LOAD_DEFAULT
		)
		{
			RenderGraphFrameAttachment attachment;
			attachment.mType = GPU::BindingFrameAttachment::RENDERTARGET;
			attachment.mLoadOperator = loadOp;
			return attachment;
		}

		static RenderGraphFrameAttachment DepthStencil(
			GPU::BindingFrameAttachment::LoadOperation loadOp = GPU::BindingFrameAttachment::LOAD_DEFAULT
		)
		{
			RenderGraphFrameAttachment attachment;
			attachment.mType = GPU::BindingFrameAttachment::DEPTH_STENCIL;
			attachment.mLoadOperator = loadOp;
			return attachment;
		}
	};
}