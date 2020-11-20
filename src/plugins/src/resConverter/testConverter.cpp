#include "resConverter.h"

namespace Cjing3D
{
	bool TestResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return true;
	}

	bool TestResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		return true;
	}
}