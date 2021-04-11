#pragma once

#include "modelImporter.h"

namespace Cjing3D
{
	class ModelImporterOBJ : public ModelImporter
	{
	public:
		ModelImporterOBJ();
		virtual ~ModelImporterOBJ();

		bool Import(ResConverterContext& context, Span<char> memBuffer, const char* src)override;
		void WriteModel(MemoryStream& stream)override;
		bool SupportsFileExt(const char* ext)override;
	};
}