#include "textureCompressor.h"

#define RGBCX_IMPLEMENTATION
#include "rgbcx\rgbcx.h"

namespace Cjing3D
{
namespace TextureCompressor
{
	// some useful compressors
	namespace
	{

	}

	bool Compress(Compressor compressor, Image& image, const Options& options, MemoryStream& dst)
	{
		return true;
	}

	bool Compress(const Image& image, const Options& options, MemoryStream& dst)
	{
		return true;
	}
}
}