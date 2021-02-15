#include "modelImporterOBJ.h"
#include "modelConverter.h"

namespace Cjing3D
{
	ModelImporterOBJ::ModelImporterOBJ()
	{
	}
	ModelImporterOBJ::~ModelImporterOBJ()
	{
	}

	bool ModelImporterOBJ::Import(ResConverterContext& context, const char* src)
	{
		return false;
	}

	const char* ModelImporterOBJ::GetOutput() const
	{
		return nullptr;
	}

	bool ModelImporterOBJ::SupportsFileExt(const char* ext)
	{
		return false;
	}
}