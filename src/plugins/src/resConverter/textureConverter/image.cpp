#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

namespace Cjing3D
{
	Image::Image(
		GPU::TEXTURE_TYPE texType, 
		GPU::FORMAT format, 
		I32 width, I32 height, I32 depth, 
		I32 level, U8* data, 
		DataFreeFunc freeFunc) :
		mTexType(texType),
		mFormat(format),
		mWidth(width),
		mHeight(height),
		mDepth(depth),
		mMipLevel(level),
		mData(data),
		mDataFreeFunc(freeFunc)
	{
		if (!data)
		{
			U32 bytes = GPU::GetTextureSize(format, width, height, depth, level);
			if (texType == GPU::TEXTURE_CUBE) {
				bytes *= 6;
			}
			
			if (bytes > 0)
			{
				mData = CJING_NEW_ARR(U8, bytes);
				Memory::Memset(mData, 0, bytes);
				mDataFreeFunc = [bytes](U8* data) {
					CJING_SAFE_DELETE_ARR(data, bytes);
				};
			}
		}
	}

	Image::Image(Image&& rhs)
	{
		Swap(rhs);
	}

	Image& Image::operator=(Image&& rhs)
	{
		Swap(rhs);
		return *this;
	}

	Image::~Image()
	{
		if (mDataFreeFunc && mData) {
			mDataFreeFunc(mData);
		}
	}

	void Image::Swap(Image& rhs)
	{
		std::swap(mTexType,  rhs.mTexType);
		std::swap(mWidth,    rhs.mWidth);
		std::swap(mHeight,   rhs.mHeight);
		std::swap(mDepth,    rhs.mDepth);
		std::swap(mMipLevel, rhs.mMipLevel);
		std::swap(mFormat,   rhs.mFormat);
		std::swap(mData,     rhs.mData);
		std::swap(mDataFreeFunc, rhs.mDataFreeFunc);
	}

	void* Image::GetMipAddr(I32 mipLevel) const
	{
		if (mipLevel > mMipLevel) {
			return nullptr;
		}

		GPU::FormatInfo formatInfo = GPU::GetFormatInfo(mFormat);
		I32 blockW = (mWidth + formatInfo.mBlockW - 1)  / formatInfo.mBlockW;
		I32 blockH = (mHeight + formatInfo.mBlockH - 1) / formatInfo.mBlockH;

		U8* targetData = mData;
		for (int i = 0; i < mMipLevel; i++)
		{
			if (i == mipLevel) {
				return targetData;
			}

			const I64 numBlocks = blockW * blockH;
			targetData += (numBlocks * formatInfo.mBlockBits) >> 3; // /8
			blockW = std::max(blockW >> 1, 1);
			blockH = std::max(blockH >> 1, 1);
		}

		return nullptr;
	}

	Image Image::Load(const char* data, size_t length, const char* ext)
	{
		if (EqualString(ext, "DDS"))
		{
			// load dds
		}
		else
		{
			const I32 channelCount = 4;
			// non-HDR imgage
			I32 width, height, bpp;
			unsigned char* rgb = stbi_load_from_memory((stbi_uc*)data, length, &width, &height, &bpp, channelCount);
			if (rgb != nullptr)
			{
				return Image(GPU::TEXTURE_2D, GPU::FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, rgb,
					[](U8* data) { stbi_image_free(data); }
				);
			}

			// HDR image
			float* rgbf = stbi_loadf_from_memory((stbi_uc*)data, length, &width, &height, &bpp, channelCount);
			if (rgbf != nullptr)
			{
				return Image(GPU::TEXTURE_2D, GPU::FORMAT_R32G32B32A32_FLOAT, width, height, 1, 1, rgb,
					[](U8* data) { stbi_image_free(data); }
				);
			}
		}

		return Image();
	}
}