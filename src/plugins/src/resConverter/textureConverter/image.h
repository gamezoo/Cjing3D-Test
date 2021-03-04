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
		~Image();

		Image(Image&& rhs);
		Image& operator=(Image&& rhs);

		static Image Load(const char* data, size_t length, const char * ext);

		void Swap(Image& rhs);

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