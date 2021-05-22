#include "textureCompressor.h"
#include "core\concurrency\jobsystem.h"
#include "renderer\textureImpl.h"

#define RGBCX_IMPLEMENTATION
#include "rgbcx\rgbcx.h"

namespace Cjing3D
{
namespace TextureCompressor
{
	class OnceInitializer
	{
	public:
		OnceInitializer()
		{
			rgbcx::init();
		}
	};

	U32 GetCompressedMipSize(U32 width, U32 height, U32 bytesPerBlock)
	{
		return ((width + 3) >> 2) * ((height + 3) >> 2) * bytesPerBlock;
	}

	U32 GetCompressedSize(U32 width, U32 height, U32 faces, U32 mips, U32 bytesPerBlock)
	{
		U32 size = GetCompressedMipSize(width, height, bytesPerBlock) * faces;
		for (int mip = 1; mip < mips; mip++)
		{
			width = std::max(1u, width >> 1);
			height = std::max(1u, height >> 1);
			size += GetCompressedMipSize(width, height, bytesPerBlock) * faces;
		}
		return size;
	}

	void ComputeMip(const U8* src, U32 srcSize, U8* dst, U32 dstSize, U32 srcW, U32 srcH, U32 dstW, U32 dstH, bool isSrgb)
	{
		PROFILE_FUNCTION();
		Debug::ThrowIfFailed(false, "ComputeMip dose not impl now.");
	}

	bool CompressRGBA(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		PROFILE_FUNCTION();
		outputStream.Write(data, size);
		return true;
	}

	bool CompressBC5(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		PROFILE_FUNCTION();

		// compressed block size: 4x4x4
		const U32 dstBlockSize = 16;
		const U32 compressedSize = GetCompressedMipSize(width, height, dstBlockSize);
		U32 offset = outputStream.Size();
		outputStream.Resize(offset + compressedSize);
		U8* dst = outputStream.data() + offset;

		JobSystem::JobHandle handle = JobSystem::INVALID_HANDLE;
		JobSystem::RunJobs((I32)height, 4,
			[&](I32 heightIndex, JobSystem::JobGroupArgs*, void*)->bool {

				U32 blockData[32];

				const U32 srcBlockHeight = std::min(height - heightIndex, 4u);
				const U8* srcRowBegin = &data[heightIndex * width * 4];
				for (int widthIndex = 0; widthIndex < width; widthIndex += 4)
				{
					const U8* srcBlockBegin = srcRowBegin + widthIndex * 4;

					// get block data
					const U32 srcBlockWidth = std::min(width - widthIndex, 4u);
					for (U32 h = 0; h < srcBlockHeight; h++) {
						Memory::Memcpy(&blockData[h * 4], &srcBlockBegin[h * width * 4], 4 * srcBlockWidth);
					}

					// write compressed block
					const U32 dstAdd = ((heightIndex >> 2) * ((width + 3) >> 2) + (widthIndex >> 2)) * dstBlockSize;
					rgbcx::encode_bc5(&dst[dstAdd], (const U8*)blockData, true, false);
				}

				return true;
			},
			0, &handle);
		JobSystem::Wait(&handle);

		return true;
	}

	bool CompressBC3(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		PROFILE_FUNCTION();

		// compressed block size: 4x4x4
		const U32 dstBlockSize = 16;
		const U32 compressedSize = GetCompressedMipSize(width, height, dstBlockSize);
		U32 offset = outputStream.Size();
		outputStream.Resize(offset + compressedSize);
		U8* dst = outputStream.data() + offset;

		JobSystem::JobHandle handle = JobSystem::INVALID_HANDLE;
		JobSystem::RunJobs((I32)height, 4,
			[&](I32 heightIndex, JobSystem::JobGroupArgs*, void*)->bool {

				U32 blockData[32];

				const U32 srcBlockHeight = std::min(height - heightIndex, 4u);
				const U8* srcRowBegin = &data[heightIndex * width * 4];
				for (int widthIndex = 0; widthIndex < width; widthIndex += 4)
				{
					const U8* srcBlockBegin = srcRowBegin + widthIndex * 4;

					// get block data
					const U32 srcBlockWidth = std::min(width - widthIndex, 4u);
					for (U32 h = 0; h < srcBlockHeight; h++) {
						Memory::Memcpy(&blockData[h * 4], &srcBlockBegin[h * width * 4], 4 * srcBlockWidth);
					}

					// write compressed block
					const U32 dstAdd = ((heightIndex >> 2) * ((width + 3) >> 2) + (widthIndex >> 2)) * dstBlockSize;
					rgbcx::encode_bc3(10, &dst[dstAdd], (const U8*)blockData);
				}

				return true;
			},
			0, &handle);
		JobSystem::Wait(&handle);

		return true;
	}

	bool CompressBC1(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		PROFILE_FUNCTION();

		// compressed block size: 4x4x4
		const U32 dstBlockSize = 8;
		const U32 compressedSize = GetCompressedMipSize(width, height, dstBlockSize);
		U32 offset = outputStream.Size();
		outputStream.Resize(offset + compressedSize);
		U8* dst = outputStream.data() + offset;

		JobSystem::JobHandle handle = JobSystem::INVALID_HANDLE;
		JobSystem::RunJobs((I32)height, 4,
			[&](I32 heightIndex, JobSystem::JobGroupArgs*, void*)->bool {

				U32 blockData[32];

				const U32 srcBlockHeight = std::min(height - heightIndex, 4u);
				const U8* srcRowBegin = &data[heightIndex * width * 4];
				for (int widthIndex = 0; widthIndex < width; widthIndex += 4)
				{
					const U8* srcBlockBegin = srcRowBegin + widthIndex * 4;

					// get block data
					const U32 srcBlockWidth = std::min(width - widthIndex, 4u);
					for (U32 h = 0; h < srcBlockHeight; h++) {
						Memory::Memcpy(&blockData[h * 4], &srcBlockBegin[h * width * 4], 4 * srcBlockWidth);
					}

					// write compressed block
					const U32 dstAdd = ((heightIndex >> 2) * ((width + 3) >> 2) + (widthIndex >> 2)) * dstBlockSize;
					rgbcx::encode_bc1(10, &dst[dstAdd], (const U8*)blockData, true, false);
				}

				return true;
			},	
			0, &handle);
		JobSystem::Wait(&handle);

		return true;
	}

	bool Compress(Compressor compressor, const Image& image, const Options& options, MemoryStream& dst, GPU::FORMAT compressedFormat)
	{
		const U32 mips = options.mIsGenerateMipmaps ? 1 + (U32)log2(std::max(image.GetWidth(), image.GetHeight())) : image.GetMipLevels();
		const U32 faces = options.mIsCubeMap ? 6 : 1;
		U32 srcWidth = image.GetWidth();
		U32 srcHeight = image.GetHeight();
		U32 blockSize = GPU::GetFormatInfo(compressedFormat).mBlockBits >> 3;

		U32 compressedSize = GetCompressedSize(srcWidth, srcHeight, faces, mips, blockSize);
		dst.Reserve(dst.Size() + compressedSize);

		DynamicArray<U8> mipData;
		DynamicArray<U8> prevMipData;

		for (U32 slice = 0; slice < image.GetSlices(); slice++)
		{
			for (U32 face = 0; face < faces; face++)
			{
				for (U32 mip = 0; mip < mips; mip++)
				{
					U32 mipWidth  = std::max(srcWidth  >> mip, 1u);
					U32 mipHeight = std::max(srcHeight >> mip, 1u);
					if (options.mIsGenerateMipmaps)
					{
						if (mip == 0)
						{
							const auto& imgData = image.GetData(slice, face, mip);
							compressor(imgData.mMem.data(), imgData.mMem.Size(), dst, mipWidth, mipHeight);
						}
						else
						{
							mipData.resize(mipWidth * mipHeight * 4);
							
							// compute mipmap for (mipWidth, mipHeight)
							U32 srcMipWidth  = std::max(srcWidth  >> (mip - 1), 1u);
							U32 srcMipHeight = std::max(srcHeight >> (mip - 1), 1u);
							if (mip == 1)
							{
								const auto& imgData = image.GetData(slice, face, 0);
								ComputeMip(imgData.mMem.data(), imgData.mMem.Size(), mipData.data(), mipData.size(), srcMipWidth, srcMipHeight, mipWidth, mipHeight, options.mIsRGB);
							}
							else
							{
								ComputeMip(prevMipData.data(), prevMipData.size(), mipData.data(), mipData.size(), srcMipWidth, srcMipHeight, mipWidth, mipHeight, options.mIsRGB);
							}

							compressor(mipData.data(), mipData.size(), dst, mipWidth, mipHeight);
							prevMipData.swap(mipData);
						}
					}
					else
					{
						const auto& imgData = image.GetData(slice, face, mip);
						compressor(imgData.mMem.data(), imgData.mMem.Size(), dst, mipWidth, mipHeight);
					}
				}
			}
		}

		return true;
	}

	bool Compress(const Image& image, const Options& options, MemoryStream& dst)
	{
		PROFILE_FUNCTION();
		static OnceInitializer initializer;

		if (!image) {
			return false;
		}

		bool canCompress = options.mIsCompress;
		canCompress &= (image.GetWidth() % 4 == 0) && (image.GetHeight() % 4 == 0);

		GPU::FORMAT format = GPU::FORMAT::FORMAT_R8G8B8A8_UNORM;
		if (canCompress)
		{
			if (options.mIsNormalMap) {
				format = GPU::FORMAT::FORMAT_BC5_UNORM;
			}
			else if (image.HasAlpha()) {
				format = GPU::FORMAT::FORMAT_BC3_UNORM;
			}
			else {
				format = GPU::FORMAT::FORMAT_BC1_UNORM;
			}
		}

		// texture lbc header
		TextureLBCHeader lbcHeader;
		lbcHeader.mCompressedFormat = format;
		dst.Write(lbcHeader);

		// compress texture
		bool ret = true;
		switch (format)
		{
		case Cjing3D::GPU::FORMAT_R8G8B8A8_UNORM:
			ret = Compress(CompressRGBA, image, options, dst, format);
			break;
		case Cjing3D::GPU::FORMAT_BC5_UNORM:
			ret = Compress(CompressBC5,  image, options, dst, format);
			break;
		case Cjing3D::GPU::FORMAT_BC3_UNORM:
			ret = Compress(CompressBC3,  image, options, dst, format);
			break;
		case Cjing3D::GPU::FORMAT_BC1_UNORM:
			ret = Compress(CompressBC1,  image, options, dst, format);
			break;
		default:
			break;
		}
		return ret;
	}
}
}