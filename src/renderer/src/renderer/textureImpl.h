#pragma once

#include "texture.h"

namespace Cjing3D
{
	static const I32 TEXTURE_MAX_NAME_LENGTH = 64;

	struct TextureGeneralHeader
	{
		static const I32 MAJOR = 0;
		static const I32 MINOR = 1;

		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;
		char mFileType[TEXTURE_MAX_NAME_LENGTH] = { '\0' };
		GPU::TextureDesc mDesc;
	};
}