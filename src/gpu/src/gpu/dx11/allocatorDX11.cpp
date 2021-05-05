#include "allocatorDX11.h"
#include "deviceDX11.h"
#include "math\hash.h"
#include "gpu\gpu.h"

#include <algorithm>

namespace Cjing3D
{
namespace GPU
{
	static constexpr size_t CACHE_CAPACITY = 64u << 20u;   // 64 MiB
	static constexpr size_t CACHE_MAX_AGE = 30u;

	ResourceAllocatorDX11::ResourceAllocatorDX11(GraphicsDeviceDx11& device) :
		mDevice(device)
	{
	}

	ResourceAllocatorDX11::~ResourceAllocatorDX11()
	{
	}

	void ResourceAllocatorDX11::Clear()
	{
		Debug::CheckAssertion(mUsedTextures.empty());
		for (auto kvp : mTextureCache) {
			mDevice.DestroyTextureImpl(kvp.second.mTexture);
		}
		mTextureCache.clear();
	}

	void ResourceAllocatorDX11::GC()
	{
		const U32 currentAge = mAge++;
		for (auto it = mTextureCache.begin(); it != mTextureCache.end();)
		{
			const U32 ageDiff = currentAge - (*it).second.mAge;
			if (ageDiff >= CACHE_MAX_AGE)
			{
				it = Purge(it);
				if (mCacheSize < CACHE_CAPACITY) {
					break;
				}
			}
			else
			{
				++it;
			}
		}

		if (mCacheSize >= CACHE_CAPACITY)
		{
			DynamicArray<std::pair<U32, TextureCachePayload>> textureCache;
			textureCache.reserve(mTextureCache.size());
			for (auto kvp : mTextureCache) {
				textureCache.push({ kvp.first, kvp.second });
			}

			// sort by least recently used
			std::sort(textureCache.begin(), textureCache.end(), 
				[](auto const& lhs, auto const& rhs) {
					return lhs.second.mAge < rhs.second.mAge;
			});

			auto curr = textureCache.begin();
			while (mCacheSize >= CACHE_CAPACITY) 
			{
				// by construction this entry must exist
				Purge(*mTextureCache.find((*curr).first));
				++curr;
			}

			// Since we're sorted already, reset the oldestAge of the whole system
			size_t oldestAge = (*textureCache.begin()).second.mAge;
			for (auto kvp : mTextureCache) {
				kvp.second.mAge -= oldestAge;
			}
			mAge -= oldestAge;
		}
	}

	TextureDX11 ResourceAllocatorDX11::CreateTexture(ResHandle handle, const TextureDesc& desc)
	{
		U32 hash = GetTextureHash(desc);
		auto it = mTextureCache.find(hash);
		if (it != nullptr)
		{
			TextureDX11 ret = it->mTexture;
			mTextureCache.erase(hash);
			mUsedTextures.insert(handle.GetHash(), desc);
			mCacheSize -= it->mSize;
			return ret;
		}
		else
		{
			TextureDX11 newTex;
			mDevice.CreateTextureImpl(newTex, &desc, nullptr);
			mUsedTextures.insert(handle.GetHash(), desc);
			return newTex;
		}
	}

	void ResourceAllocatorDX11::DestroyTexture(ResHandle handle, TextureDX11& texture)
	{
		auto it = mUsedTextures.find(handle.GetHash());
		Debug::CheckAssertion(it != nullptr);

		U32 hash = GetTextureHash(*it);
		U32 texSize = GPU::GetTextureSize(it->mFormat, it->mWidth, it->mHeight, it->mDepth, it->mMipLevels);
		mTextureCache.insert(hash, TextureCachePayload{ texture, mAge, texSize });
		mCacheSize += texSize;

		mUsedTextures.erase(handle.GetHash());
	}

	U32 ResourceAllocatorDX11::GetTextureHash(const TextureDesc& desc) const
	{
		U32 hash = 0;
		HashCombine(hash, desc.mFormat);
		HashCombine(hash, desc.mWidth);
		HashCombine(hash, desc.mHeight);
		HashCombine(hash, desc.mDepth);
		HashCombine(hash, desc.mArraySize);
		HashCombine(hash, desc.mMipLevels);
		HashCombine(hash, desc.mSampleCount);
		HashCombine(hash, desc.mType);
		HashCombine(hash, desc.mUsage);
		HashCombine(hash, desc.mBindFlags);
		HashCombine(hash, desc.mCPUAccessFlags);
		HashCombine(hash, desc.mMiscFlags);
		return hash;
	}

	HashMap<U32, ResourceAllocatorDX11::TextureCachePayload>::Iterator ResourceAllocatorDX11::Purge(HashMap<U32, TextureCachePayload>::Iterator& it)
	{
		mDevice.DestroyTextureImpl((*it).second.mTexture);
		mCacheSize -= (*it).second.mSize;
		return mTextureCache.erase(it);
	}

	void ResourceAllocatorDX11::Purge(TextureCachePayload& texture)
	{
		mDevice.DestroyTextureImpl(texture.mTexture);
		mCacheSize -= texture.mSize;
	}
}
}