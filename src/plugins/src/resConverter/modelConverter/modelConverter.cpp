#include "modelConverter.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "core\helper\stream.h"
#include "renderer\modelImpl.h"

#include "modelImporterOBJ.h"

namespace Cjing3D
{
	void ModelMetaObject::Serialize(JsonArchive& archive)const
	{

	}

	void ModelMetaObject::Unserialize(JsonArchive& archive) 
	{

	}

	ModelResConverter::ModelResConverter()
	{
		RegisterImporter("obj", CJING_NEW(ModelImporterOBJ));
	}

	bool ModelResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		ModelMetaObject data = context.GetMetaData<ModelMetaObject>();
		BaseFileSystem& fileSystem = context.GetFileSystem();

		// 1. get taget model importer
		MaxPathString srcExt;
		Path::GetPathExtension(Span(src, StringLength(src)), srcExt.toSpan());
		ModelImporter* importer = GetImporter(srcExt);
		if (importer) {
			return false;
		}

		// 2. import model
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);

		if (!importer->Import(context, Span(source.data(), source.size()), src)) {
			return false;
		}

		// 3. write
		MemoryStream stream;
		importer->WriteModel(stream);

		// write resource
		if (!context.WriteResource(dest, stream.data(), stream.Size())) {
			return false;
		}

		context.AddOutput(dest);
		context.SetMetaData<ModelMetaObject>(data);

		return true;
	}

	void ModelResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
	}

	bool ModelResConverter::SupportsFileExt(const char* ext)
	{
		return mImporters.find(ext) != nullptr;
	}

	bool ModelResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Model") && SupportsFileExt(ext);
	}
}