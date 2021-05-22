#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	class TextureMetaObject : public SerializedObject
	{
	public:
		virtual void Serialize(JsonArchive& archive)const;
		virtual void Unserialize(JsonArchive& archive);

		GPU::FORMAT mFormat = GPU::FORMAT_UNKNOWN;
		bool mGenerateMipmap = false;
		bool mIsNormalMap = false;
		F32 mMipScale = -1.0f;

		enum WrapMode
		{
			REPEAT,
			CLAMP
		};
		enum Filter 
		{
			LINEAR,
			POINT,
			ANISOTROPIC
		};

		Filter mFilter = Filter::LINEAR;
		WrapMode mWrapModeU = WrapMode::REPEAT;
		WrapMode mWrapModeV = WrapMode::REPEAT;
		WrapMode mWrapModeW = WrapMode::REPEAT;
	};

	class TextureResConverter : public IResConverter
	{
	public:
		void OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)override;
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	
	private:
		GPU::ResHandle mCurrentTexture;
		TextureMetaObject mCurrentTextureMeta;
	};
}