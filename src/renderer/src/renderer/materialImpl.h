#pragma once

#include "material.h"

namespace Cjing3D
{
	static const I32 MATERIAL_MAX_NAME_LENGTH = 64;

	struct MaterialTexture
	{
		char mPath[MATERIAL_MAX_NAME_LENGTH] = { '\0' };
	};

	struct MaterialData
	{
		char mShaderPath[MATERIAL_MAX_NAME_LENGTH] = { '\0' };
		Material::ShaderType mShaderType = Material::ShaderType_Standard;
		MaterialTexture mTextures[Material::TextureSlot_Count];
	};

	class MaterialImpl
	{
	public:
		MaterialImpl() {}
		~MaterialImpl() {}

		GPU::ResHandle GetTexture(Material::TextureSlot slot)const {
			return mTextures[(I32)slot].mTexture ? 
				mTextures[(I32)slot].mTexture->GetHandle() : GPU::ResHandle::INVALID_HANDLE;
		}

	private:
		friend class Material;
		friend class MaterialFactory;

		MaterialData mData;
		struct TextureMapSlot
		{
			String mName;
			TextureRef mTexture;
		};
		TextureMapSlot mTextures[Material::TextureSlot_Count];
		Material::ShaderType mShaderType = Material::ShaderType_Standard;
		ShaderRef mShader;
	};
}