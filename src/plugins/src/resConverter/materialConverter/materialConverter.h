#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"

namespace Cjing3D
{
	class MaterialResConverter : public IResConverter
	{
	public:
		void OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)override;
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	};
}