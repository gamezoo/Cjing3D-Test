#pragma once

#include "core\filesystem\file.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	class Image
	{
	public:
		using DataFreeFunc = Function<void(U8* data)>;

		Image() = default;
		Image(GPU::TEXTURE_TYPE texType, GPU::FORMAT format, I32 width, I32 height, I32 depth, I32 level, U8* data, DataFreeFunc freeFunc);
		Image(Image&& rhs);
		Image& operator=(Image&& rhs);
		~Image();

		void Swap(Image& rhs);
		GPU::FORMAT GetFormat()const { return mFormat; }
		void* GetMipAddr(I32 mipLevel) const;

		template<typename T>
		T* GetMipData(I32 mipLevel)
		{
			return reinterpret_cast<T*>(GetMipAddr(mipLevel));
		}

		explicit operator bool()const {
			return mData != nullptr;
		}

		static Image Load(const char* data, size_t length, const char* ext);

	private:
		Image(const Image& rhs) = delete;
		Image& operator=(const Image& rhs) = delete;

		DataFreeFunc mDataFreeFunc = nullptr;
		GPU::TEXTURE_TYPE mTexType = GPU::TEXTURE_TYPE::TEXTURE_2D;
		GPU::FORMAT mFormat = GPU::FORMAT_UNKNOWN;
		I32 mWidth = 0;
		I32 mHeight = 0;
		I32 mDepth = 0;
		I32 mMipLevel = 0;
		U8* mData = nullptr;
		DataFreeFunc freeFunc = nullptr;
	};
}