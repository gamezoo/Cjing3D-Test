#include "textureCompressor.h"

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

	bool CompressRGBA(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		return true;
	}

	bool CompressBC5(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		return true;
	}

	bool CompressBC3(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		return true;
	}

	bool CompressBC1(const U8* data, U32 size, MemoryStream& outputStream, U32 width, U32 height)
	{
		return true;
	}

	bool Compress(Compressor compressor, const Image& image, const Options& options, MemoryStream& dst)
	{
		const U32 mips = options.mIsGenerateMipmaps ? 1 + (U32)log2(std::max(image.GetWidth(), image.GetHeight())) : image.GetMipLevels();

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

		bool ret = true;
		switch (format)
		{
		case Cjing3D::GPU::FORMAT_R8G8B8A8_UNORM:
			ret = Compress(CompressRGBA, image, options, dst);
			break;
		case Cjing3D::GPU::FORMAT_BC5_UNORM:
			ret = Compress(CompressBC5, image, options, dst);
			break;
		case Cjing3D::GPU::FORMAT_BC3_UNORM:
			ret = Compress(CompressBC3, image, options, dst);
			break;
		case Cjing3D::GPU::FORMAT_BC1_UNORM:
			ret = Compress(CompressBC1, image, options, dst);
			break;
		default:
			break;
		}
		return ret;
	}
}
}