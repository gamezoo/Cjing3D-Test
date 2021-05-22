#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

namespace Cjing3D
{
	Image::Image(
		GPU::TEXTURE_TYPE texType, 
		GPU::FORMAT format, 
		U32 width, U32 height, U32 depth, U32 mips, U32 slices,
		U8* data,
		DataFreeFunc freeFunc) :
		mTexType(texType),
		mFormat(format),
		mWidth(width),
		mHeight(height),
		mDepth(depth),
		mMips(mips),
		mSlices(slices),
		mSrcData(data),
		mDataFreeFunc(freeFunc)
	{
		GPU::FormatInfo formatInfo = GPU::GetFormatInfo(format);
		mHasAlpha = formatInfo.mABits > 0;

		AddDatas(0, 0, data);
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
		if (mDataFreeFunc && mSrcData)
		{
			mDataFreeFunc(mSrcData);
		}
	}

	void Image::Swap(Image& rhs)
	{
		std::swap(mTexType,  rhs.mTexType);
		std::swap(mWidth,    rhs.mWidth);
		std::swap(mHeight,   rhs.mHeight);
		std::swap(mDepth,    rhs.mDepth);
		std::swap(mMips,     rhs.mMips);
		std::swap(mSlices,   rhs.mSlices);
		std::swap(mFormat,   rhs.mFormat);
		std::swap(mDatas,    rhs.mDatas);
		std::swap(mDataFreeFunc, rhs.mDataFreeFunc);
	}

	const Image::ImageData& Image::GetData(U32 slice, U32 face, U32 mipLevel)const
	{
		for (auto& imgData : mDatas)
		{
			if (imgData.mSlice == slice && imgData.mMip == mipLevel && imgData.mFace == face) {
				return imgData;
			}
		}
		Debug::ThrowIfFailed(false);
		return mDatas[0];
	}


	Image Image::Load(const char* data, size_t length, const char* ext)
	{
		if (EqualString(ext, "DDS"))
		{
		}
		else
		{
			const U32 channelCount = 4;
			I32 width, height, bpp;
			GPU::FORMAT format = GPU::FORMAT_R8G8B8A8_UNORM;

			// non-hdr
			U8* rgb = (U8*)stbi_load_from_memory((stbi_uc*)data, length, &width, &height, &bpp, channelCount);
			if (rgb == nullptr)
			{
				// hdr
				format = GPU::FORMAT_R32G32B32A32_FLOAT;
				rgb = (U8*)stbi_loadf_from_memory((stbi_uc*)data, length, &width, &height, &bpp, channelCount);
				if (rgb == nullptr) 
				{
					Logger::Warning("Failed to load image from memory");
					return Image();
				}
			}

			return Image(
				GPU::TEXTURE_2D, 
				format,
				(U32)width, 
				(U32)height, 
				1, 
				1, 
				1,
				rgb,
				[](U8* data) {
					stbi_image_free(data); 
				}
			);
		}

		return Image();
	}

	void Image::AddDatas(U32 slice, U32 face, U8* data)
	{
		GPU::FormatInfo formatInfo = GPU::GetFormatInfo(mFormat);
		U32 blockW = (mWidth + formatInfo.mBlockW - 1)  / formatInfo.mBlockW;
		U32 blockH = (mHeight + formatInfo.mBlockH - 1) / formatInfo.mBlockH;

		U8* targetData = data;
		for (int mip = 0; mip < mMips; mip++)
		{
			const size_t numBlocks = (size_t)(blockW * blockH);
			const size_t size = (numBlocks * formatInfo.mBlockBits) >> 3;	
			AddData(slice, face, mip, targetData, size);

			targetData += size;
			blockW = std::max(blockW >> 1, 1u);
			blockH = std::max(blockH >> 1, 1u);
		}
	}

	void Image::AddData(U32 slice, U32 face, U32 mip, U8* data, size_t size)
	{
		if (mip > mMips) {
			return;
		}

		ImageData& imgData = mDatas.emplace();
		imgData.mSlice = slice;
		imgData.mFace = face;
		imgData.mMip = mip;
		imgData.mMem = InputMemoryStream(data, size);
	}
}