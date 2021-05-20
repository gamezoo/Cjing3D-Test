#pragma once

#include "texture.h"

namespace Cjing3D
{
	static const I32 TEXTURE_MAX_NAME_LENGTH = 64;

	enum class TextureFlags : U32
	{
		CLAMP_U = 1 << 0,
		CLAMP_V = 1 << 1,
		CLAMP_W = 1 << 2,
	};

	struct TextureGeneralHeader
	{
		static const I32 MAJOR = 0;
		static const I32 MINOR = 1;

		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;
		char mFileType[TEXTURE_MAX_NAME_LENGTH] = { '\0' };
		GPU::TextureDesc mTexDesc;
		U32 mTexFlags = 0;
	};
}