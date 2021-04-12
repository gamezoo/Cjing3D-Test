#pragma once

#include "model.h"
#include "core\helper\stream.h"

namespace Cjing3D
{
	static const I32 MODEL_MAX_NAME_LENGTH = 64;
	static const I32 MODEL_MATERIAL_PATH_LENGTH = 128;

	struct ModelGeneralHeader
	{
		static const U32 MAGIC;
		static const I32 MAJOR = 1;
		static const I32 MINOR = 2;

		U32 mMagic = MAGIC;
		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;

		I32 mNumMeshes = 0;
		I32 mNumMeshInstDatas = 0;
	};

	struct ModelMeshData
	{
		char mName[MODEL_MAX_NAME_LENGTH] = { '\0' };
		I32 mVertexSize = 0;
		I32 mVertices = 0;
		I32 mIndices = 0;
		I32 mStartVertexElements = 0;
		I32 mNumVertexElements = 0;
		I32 mStartSubMeshes = 0;
		I32 mNumSubMeshes = 0;
	};

	struct MeshInstData
	{
		char mMaterial[MODEL_MATERIAL_PATH_LENGTH] = { '\0' };
		I32 mMeshIndex = -1;
		I32 mSubMeshIndex = -1;
	};
}