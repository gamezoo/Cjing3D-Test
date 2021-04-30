#pragma once

#include "includeDX11.h"
#include "resourceDX11.h"

namespace Cjing3D
{
namespace GPU
{
	class ResourceAllocatorDX11
	{
	public:
		void GC();

		bool CreateTexture(ResHandle handle, const TextureDesc* desc);
		void DestroyTexture(ResHandle handle);
	};
}
}