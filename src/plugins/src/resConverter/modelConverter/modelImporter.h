#pragma once

#include "resource\converter.h"
#include "core\helper\stream.h"

namespace Cjing3D
{
	class ModelResConverter;

	class ModelImporter
	{
	public:
		ModelImporter() {};
		virtual ~ModelImporter() {}

		virtual bool Import(ResConverterContext& context, Span<char> memBuffer, const char* src) = 0;
		virtual bool WriteModel(MemoryStream& stream) = 0;
		virtual bool SupportsFileExt(const char* ext) = 0;
	};
}