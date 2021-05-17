#pragma once

#include "core\common\common.h"
#include "core\helper\stream.h"
#include "image.h"

namespace Cjing3D
{
namespace TextureCompressor
{
	using Compressor = Function<void(U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)>;

	struct Options
	{
		bool mIsCompress = true;
		bool mIsGenerateMipmaps = false;
	};

	bool Compress(Compressor compressor, const Image& image, const Options& options, MemoryStream& dst);
	bool Compress(const Image& image, const Options& options, MemoryStream& dst);
}
}