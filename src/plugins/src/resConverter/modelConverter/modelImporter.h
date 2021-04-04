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

		virtual bool Import(ResConverterContext& context, Span<char> memBuffer, const char* src) = 0;
		virtual void WriteModel(File& file) = 0;
		virtual bool SupportsFileExt(const char* ext) = 0;
	};
}