#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"

namespace Cjing3D
{
	class ShaderMetaObject : public SerializedObject
	{
	public:
		

	public:
		virtual void Serialize(JsonArchive& archive);
		virtual void Unserialize(JsonArchive& archive)const;
	};

	class ShaderResConverter : public IResConverter
	{
	public:
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	};
}