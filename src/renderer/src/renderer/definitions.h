#pragma once

#include "gpu\definitions.h"
#include "gpu\resource.h"
#include "math\viewport.h"

#define CJING_SHADER_INTEROP
#include "shaderInterop\shaderInterop.h"
#include "shaderInterop\shaderInteropRender.h"

namespace Cjing3D
{
	enum BLENDMODE
	{
		BLENDMODE_OPAQUE,
		BLENDMODE_ALPHA,
		BLENDMODE_PREMULTIPLIED,
		BLENDMODE_ADDITIVE,
		BLENDMODE_COUNT
	};

	enum SHADERTYPE
	{
		SHADERTYPE_MAIN,
		SHADERTYPE_IMAGE,
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