#include "image.h"
#include "stb\stb_image_include.h"

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
}