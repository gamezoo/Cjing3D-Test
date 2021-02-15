#pragma once

#include "resource\converter.h"

namespace Cjing3D
{
	class ModelResConverter;

	class ModelImporter
	{
	public:
		ModelImporter() {};
		virtual ~ModelImporter() {}

		virtual bool Import(ResConverterContext& context, const char* src) = 0;
		virtual const char* GetOutput()const = 0;
		virtual bool SupportsFileExt(const char* ext) = 0;
	};
}