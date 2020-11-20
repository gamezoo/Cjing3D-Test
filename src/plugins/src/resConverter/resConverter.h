#pragma once

#include "resource\converter.h"

namespace Cjing3D
{
	// test res converter
	class TestResConverter : public IResConverter
	{
	public:
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	};
}