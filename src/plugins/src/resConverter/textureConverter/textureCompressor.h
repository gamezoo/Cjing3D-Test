#pragma once

#include "core\common\common.h"
#include "core\helper\stream.h"
#include "image.h"

namespace Cjing3D
{
namespace TextureCompressor
{
	using Compressor = Function<bool(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)>;

	struct Options
	{
		bool mIsCompress = true;
		bool mIsGenerateMipmaps = false;
		bool mIsNormalMap = false;
		bool mIsRGB = false;
		bool mIsCubeMap = false;
	};

	bool CompressRGBA(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height);
	bool CompressBC5(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height);
	bool CompressBC3(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height);
	bool CompressBC1(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height);

	bool Compress(Compressor compressor, const Image& image, const Options& options, MemoryStream& dst, GPU::FORMAT compressedFormat);
	bool Compress(const Image& image, const Options& options, MemoryStream& dst);
}
}