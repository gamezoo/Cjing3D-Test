#include "materialConverter.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{
	void MaterialMetaObject::Serialize(JsonArchive& archive)const
	{

	}

	void MaterialMetaObject::Unserialize(JsonArchive& archive) 
	{

	}

	bool MaterialResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		MaterialMetaObject data = context.GetMetaData<MaterialMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);


		context.AddOutput(dest);
		context.SetMetaData<MaterialMetaObject>(data);
		return true;
	}

    bool MaterialResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Material") && EqualString(ext, "jmat");
	}

	LUMIX_PLUGIN_ENTRY(materialConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(MaterialResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}