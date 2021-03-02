#include "textureConverter.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{
	void TextureMetaObject::Serialize(JsonArchive& archive)const
	{

	}

	void TextureMetaObject::Unserialize(JsonArchive& archive) 
	{

	}

	bool TextureResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		TextureMetaObject data = context.GetMetaData<TextureMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);


		context.AddOutput(dest);
		context.SetMetaData<TextureMetaObject>(data);
		return true;
	}

	void TextureResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
	}

	bool TextureResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Texture");
	}

	LUMIX_PLUGIN_ENTRY(textureConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(TextureResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}