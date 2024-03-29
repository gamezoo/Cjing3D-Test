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
		bool WriteModel(ResConverterContext& context, MemoryStream& stream)override;
		bool WriteMaterials(ResConverterContext& context, const char* dirPath)override;
		bool SupportsFileExt(const char* ext)override;

	private:
		class ModelImporterOBJImpl* mImpl = nullptr;
	};
}