#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"

namespace Cjing3D
{
	class TextureMetaObject : public SerializedObject
	{
	public:
		virtual void Serialize(JsonArchive& archive)const;
		virtual void Unserialize(JsonArchive& archive);
	};

	class TextureResConverter : public IResConverter
	{
	public:
		void OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)override;
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	};
}