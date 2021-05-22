#pragma once

#include "core\filesystem\file.h"
#include "core\helper\stream.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	class Image
	{
	public:
		using DataFreeFunc = Function<void(U8* data)>;

		struct ImageData
		{
			InputMemoryStream mMem;
			U32 mMip = 0;
			U32 mFace = 0;
			U32 mSlice = 0;
		};

		Image() = default;
		Image(GPU::TEXTURE_TYPE texType, GPU::FORMAT format, U32 width, U32 height, U32 depth, U32 mips, U32 slices, U8* data, DataFreeFunc freeFunc);
		Image(Image&& rhs);
		Image& operator=(Image&& rhs);
		~Image();

		void  Swap(Image& rhs);

		GPU::FORMAT GetFormat()const { return mFormat; }
		U32 GetWidth()const { return mWidth; }
		U32 GetHeight()const { return mHeight; }
		U32 GetDepth()const { return mDepth; }
		U32 GetMipLevels()const { return mMips; }
		U32 GetSlices()const { return mSlices; }
		bool HasAlpha()const { return mHasAlpha; }
		bool IsCubemap()const { return mIsCubemap; }

		const ImageData& GetData(U32 slice, U32 face, U32 mipLevel)const;

		explicit operator bool()const {
			return !mDatas.empty() && mSrcData != nullptr;
		}

		static Image Load(const char* data, size_t length, const char* ext);

	private:
		Image(const Image& rhs) = delete;
		Image& operator=(const Image& rhs) = delete;

		void AddDatas(U32 slice, U32 face, U8* data);
		void AddData(U32 slice, U32 face, U32 mip, U8* data, size_t size);

	private:
		DataFreeFunc mDataFreeFunc = nullptr;
		GPU::TEXTURE_TYPE mTexType = GPU::TEXTURE_TYPE::TEXTURE_2D;
		GPU::FORMAT mFormat = GPU::FORMAT_UNKNOWN;
		U32 mWidth = 0;
		U32 mHeight = 0;
		U32 mDepth = 0;
		U32 mMips = 0;
		U32 mSlices = 0;
		bool mHasAlpha = true;
		bool mIsCubemap = false;

		U8* mSrcData = nullptr;
		DataFreeFunc freeFunc = nullptr;
		DynamicArray<ImageData> mDatas;
	};
}