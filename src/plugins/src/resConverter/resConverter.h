#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"
#include "core\container\hashMap.h"

namespace Cjing3D
{
	class ResConverter : public IResConverter
	{
	public:
		ResConverter();
		~ResConverter();

		void AddConverter(const ResourceType& type, IResConverter* converter);
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	
	private:
		HashMap<U32, IResConverter*> mResConverters;
	};
}