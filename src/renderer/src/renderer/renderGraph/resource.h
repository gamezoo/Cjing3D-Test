#pragma once

#include "renderer\definitions.h"

namespace Cjing3D
{
	class RenderGraphResource;

	enum RenderGraphQueueFlag
	{
		RENDER_GRAPH_QUEUE_GRAPHICS_BIT = 1 << 0,
		RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT = 1 << 1,
	};
	using RenderGraphQueueFlags = U32;

	// TODO: is too similar to ResourceInst and it is cost too much mem.
	struct RenderGraphResourceDimension
	{
		StaticString<64> mName;
		GPU::ResourceType mType;
		GPU::TextureDesc mTexDesc;
		GPU::BufferDesc mBufferDesc;
		bool mIsTransient = false;
		bool mPersistent = true;
		bool mIsImported = false;
		I32  mResIndex = 0;
	};

	struct RenderGraphAttachment
	{
		GPU::BindingFrameAttachment::TYPE mType = GPU::BindingFrameAttachment::RENDERTARGET;
		GPU::BindingFrameAttachment::LoadOp mLoadOp = GPU::BindingFrameAttachment::LOAD_DEFAULT;

		I32 mSubresourceIndex = -1;

		static RenderGraphAttachment RenderTarget(
			GPU::BindingFrameAttachment::LoadOp loadOp = GPU::BindingFrameAttachment::LOAD_DEFAULT
		)
		{
			RenderGraphAttachment attachment;
			attachment.mType = GPU::BindingFrameAttachment::RENDERTARGET;
			attachment.mLoadOp = loadOp;
			return attachment;
		}

		static RenderGraphAttachment DepthStencil(
			GPU::BindingFrameAttachment::LoadOp loadOp = GPU::BindingFrameAttachment::LOAD_DEFAULT
		)
		{
			RenderGraphAttachment attachment;
			attachment.mType = GPU::BindingFrameAttachment::DEPTH_STENCIL;
			attachment.mLoadOp = loadOp;
			return attachment;
		}
	};

	class RenderGraphResource
	{
	public:
		RenderGraphResource() = default;
		RenderGraphResource(I32 index) :mIndex(index) {}

		bool operator==(const RenderGraphResource& rhs) const {
			return mIndex == rhs.mIndex;
		}
		bool operator!=(const RenderGraphResource& rhs) const {
			return mIndex != rhs.mIndex;
		}
		bool operator< (const RenderGraphResource& rhs) const {
			return mIndex < rhs.mIndex;
		}
		operator bool()const {
			return mIndex != -1;
		}

		bool IsEmpty()const {
			return mIndex == -1;
		}
		I32 Index()const {
			return mIndex;
		}

		I32 mIndex = -1;
	};

	U32 HashFunc(U32 Input, const RenderGraphResource& Data);
	U64 HashFunc(U64 Input, const RenderGraphResource& Data);
}
