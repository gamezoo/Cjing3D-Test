#pragma once

#include "model.h"

namespace Cjing3D
{
	static const I32 MODEL_MAX_NAME_LENGTH = 64;

	struct ModelGeneralHeader
	{
		static const U32 MAGIC;
		static const I32 MAJOR = 1;
		static const I32 MINOR = 2;

		U32 mMagic = MAGIC;
		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;
	};
}