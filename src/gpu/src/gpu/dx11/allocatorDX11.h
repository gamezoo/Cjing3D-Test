#pragma once

#include "includeDX11.h"
#include "resourceDX11.h"
#include "core\container\hashMap.h"

namespace Cjing3D
{
namespace GPU
{
	class GraphicsDeviceDx11;

	class ResourceAllocatorDX11
	{
	public:
		ResourceAllocatorDX11(GraphicsDeviceDx11& device);
		~ResourceAllocatorDX11();

		void Clear();
		void GC();

		TextureDX11 CreateTexture(ResHandle handle, const TextureDesc& desc);
		void DestroyTexture(ResHandle handle, TextureDX11& texture);

	private:
		U32 GetTextureHash(const TextureDesc& desc)const;
		
		struct TextureCachePayload 
		{
			TextureDX11 mTexture;
			size_t mAge = 0;
			U32 mSize = 0;
		};
		HashMap<U32, TextureCachePayload> mTextureCache;
		HashMap<U32, TextureDesc> mUsedTextures;

		HashMap<U32, TextureCachePayload>::Iterator Purge(HashMap<U32, TextureCachePayload>::Iterator& it);
		void Purge(TextureCachePayload& texture);

	private:
		U32 mCacheSize = 0;
		U32 mAge = 0;
		GraphicsDeviceDx11& mDevice;
	};
}
}