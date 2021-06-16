#pragma once

#include "gpu\definitions.h"
#include "gpu\resource.h"
#include "math\viewport.h"
#include "core\scene\ecs.h"

namespace Cjing3D
{
	enum RENDERPASS
	{
		RENDERPASS_MAIN,
		RENDERPASS_PREPASS,
		RENDERPASS_SHADOW,
		RENDERPASS_COUNT
	};

	enum RENDERTYPE
	{
		RENDERTYPE_OPAQUE,
		RENDERTYPE_TRANSPARENT,
		RENDERTYPE_COUNT,
	};

	enum BLENDMODE
	{
		BLENDMODE_OPAQUE,
		BLENDMODE_ALPHA,
		BLENDMODE_PREMULTIPLIED,
		BLENDMODE_ADDITIVE,
		BLENDMODE_COUNT
	};

	enum SHADERTYPE
	{
		SHADERTYPE_MAIN,
		SHADERTYPE_IMAGE,
		SHADERTYPE_COUNT
	};

	enum CBTYPE
	{
		CBTYPE_FRAME,
		CBTYPE_CAMERA,
		CBTYPE_COUNT
	};

	enum CULLING_FLAG
	{
		CULLING_FLAG_EMPTY = 0,
		CULLING_FLAG_OBJECTS = 1,
		CULLING_FLAG_LIGHT = 1 << 1,

		CULLING_FLAG_ALL = ~0u,
	};

	struct ShaderTechHasher
	{
		RENDERPASS mRenderPass;
		BLENDMODE mBlendMode;

		U32 GetHash()const
		{
			U32 hash = 0;
			HashCombine(hash, mRenderPass);
			HashCombine(hash, mBlendMode);
			return hash;
		}
	};

	struct Visibility
	{
		Viewport* mViewport = nullptr;
		DynamicArray<ECS::Entity> mCulledObjects;
		volatile I32 mObjectCount = 0;

		void Clear()
		{
			mCulledObjects.clear();
		}
	};
} 