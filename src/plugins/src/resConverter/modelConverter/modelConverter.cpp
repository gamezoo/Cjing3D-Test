#include "modelConverter.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "renderer\model.h"

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
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);

		// 1. get taget model importer
		MaxPathString srcExt;
		Path::GetPathExtension(Span(src, StringLength(src)), srcExt.toSpan());
		ModelImporter* importer = GetImporter(srcExt);
		if (importer == nullptr) {
			return false;
		}

		// 2. import model by importer
		if (!importer->Import(context, src)) {
			return false;
		}

		context.AddOutput(dest);
		context.SetMetaData<ModelMetaObject>(data);

		return true;
	}

	bool ModelResConverter::SupportsFileExt(const char* ext)
	{
		return mImporters.find(ext) != nullptr;
	}

	bool ModelResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Model") && SupportsFileExt(ext);
	}

	LUMIX_PLUGIN_ENTRY(modelConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(ModelResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}