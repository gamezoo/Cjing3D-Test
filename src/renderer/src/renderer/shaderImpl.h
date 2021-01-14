#pragma once

#include "gpu\definitions.h"
#include "gpu\resource.h"

namespace Cjing3D
{
	struct ShaderBytecodeHeader
	{
		GPU::SHADERSTAGES mStage = GPU::SHADERSTAGES_COUNT;
		I32 mOffset = 0;
		I32 mBytes = 0;
	};

	struct ShaderTechniqueHeader
	{
		I32 mIdxVS = 0;
		I32 mIdxGS = 0;
		I32 mIdxHS = 0;
		I32 mIdxDS = 0;
		I32 mIdxPS = 0;
		I32 mIdxCS = 0;
	};

	struct ShaderGeneralHeader
	{
		static const U32 MAGIC;
		static const I32 MAJOR = 0;
		static const I32 MINOR = 1;

		U32 mMagic = MAGIC;
		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;

		I32 mNumShaders = 0;
		I32 mNumTechniques = 0;
	};

	struct ShaderImpl
	{
	public:
		String mName;
		ShaderGeneralHeader mGeneralHeader;
		DynamicArray<ShaderBytecodeHeader> mBytecodeHeaders;
		DynamicArray<ShaderTechniqueHeader> mTechniques;
		DynamicArray<char> mBytecodes;

		DynamicArray<GPU::ResHandle> mRhiShaders;

	public:
		ShaderImpl();
		~ShaderImpl();
	};
}