#pragma once

#include "modelImporter.h"

namespace Cjing3D
{
	class ModelImporterOBJ : public ModelImporter
	{
	public:
		ModelImporterOBJ();
		virtual ~ModelImporterOBJ();

		bool Import(ResConverterContext& context, const char* src)override;
		const char* GetOutput()const override;
		bool SupportsFileExt(const char* ext)override;
	};
}