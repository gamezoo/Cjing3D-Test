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
		BindingBuffer ConstantBuffer(ResHandle handle, SHADERSTAGES stage, I32 slot)
		{
			BindingBuffer binding;

			return binding;
		}

		BindingBuffer VertexBuffer(ResHandle handle, I32 offset, I32 stride)
		{
			BindingBuffer binding;

			return binding;
		}

		BindingBuffer IndexBuffer(ResHandle handle, I32 offset) 
		{
			BindingBuffer binding;

			return binding;
		}

		BindingSRV Texture(ResHandle handle, SHADERSTAGES stage, I32 slot) 
		{
			BindingSRV binding;

			return binding;
		}

		BindingSRV Buffer(ResHandle handle, SHADERSTAGES stage, I32 slot)
		{
			BindingSRV binding;

			return binding;
		}

		BindingUAV RWTexture(ResHandle handle, SHADERSTAGES stage, I32 slot)
		{
			BindingUAV binding;

			return binding;
		}

		BindingSAM Sampler(ResHandle handle, SHADERSTAGES stage, I32 slot)
		{
			return BindingSAM();
		}

		BindingUAV RWBuffer(ResHandle handle, SHADERSTAGES stage, I32 slot)
		{
			BindingUAV binding;

			return binding;
		}
	}
}
}

