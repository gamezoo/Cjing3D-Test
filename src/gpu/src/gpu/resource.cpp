#include "resource.h"
#include "gpu.h"

namespace Cjing3D
{
namespace GPU
{
	ResHandle ResHandle::INVALID_HANDLE;

	bool ResHandle::IsValid() const
	{
		return GPU::IsHandleValid(*this);
	}

	namespace Binding
	{
		BindingBuffer ConstantBuffer(ResHandle handle, SHADERSTAGES stage)
		{
			BindingBuffer binding;
			binding.mResource = handle;
			binding.mStage = stage;
			return binding;
		}

		BindingBuffer VertexBuffer(ResHandle handle, I32 offset, I32 stride)
		{
			BindingBuffer binding;
			binding.mResource = handle;
			binding.mOffset = offset;
			binding.mStride = stride;
			return binding;
		}

		BindingBuffer IndexBuffer(ResHandle handle, I32 offset) 
		{
			BindingBuffer binding;
			binding.mResource = handle;
			binding.mOffset = offset;
			return binding;
		}

		BindingSRV Texture(ResHandle handle, SHADERSTAGES stage, I32 subresourceIndex)
		{
			BindingSRV binding;
			binding.mResource = handle;
			binding.mStage = stage;
			binding.mSubresourceIndex = subresourceIndex;
			return binding;
		}

		BindingSRV Buffer(ResHandle handle, SHADERSTAGES stage)
		{
			BindingSRV binding;
			binding.mResource = handle;
			binding.mStage = stage;
			return binding;
		}

		BindingUAV RWTexture(ResHandle handle, SHADERSTAGES stage, I32 subresourceIndex)
		{
			BindingUAV binding;
			binding.mResource = handle;
			binding.mStage = stage;
			binding.mSubresourceIndex = subresourceIndex;
			return binding;
		}

		BindingSAM Sampler(ResHandle handle, SHADERSTAGES stage)
		{
			BindingSAM binding;
			binding.mResource = handle;
			binding.mStage = stage;
			return binding;
		}

		BindingUAV RWBuffer(ResHandle handle, SHADERSTAGES stage)
		{
			BindingUAV binding;
			binding.mResource = handle;
			binding.mStage = stage;
			return binding;
		}
	}
}
}

