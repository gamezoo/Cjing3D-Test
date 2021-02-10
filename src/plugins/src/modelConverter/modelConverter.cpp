#include "modelConverter.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "renderer\model.h"

namespace Cjing3D
{
	void ModelMetaObject::Serialize(JsonArchive& archive)const
	{

	}

	void ModelMetaObject::Unserialize(JsonArchive& archive) 
	{

	}

	class ModelResConverter : public IResConverter
	{
	private:

	public:
		bool SupportsFileExt(const char* ext);
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	};

	bool ModelResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		ModelMetaObject data = context.GetMetaData<ModelMetaObject>();
		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);




		context.AddOutput(dest);
		context.SetMetaData<ModelMetaObject>(data);

		return true;
	}

	bool ModelResConverter::SupportsFileExt(const char* ext)
	{
		return false;
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