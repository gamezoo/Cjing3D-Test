#include "resConverter.h"
#include "shaderConverter\shaderConverter.h"
#include "modelConverter\modelConverter.h"

namespace Cjing3D
{
	ResConverter::ResConverter()
	{
		AddConverter(ResourceType("Shader"), CJING_NEW(ShaderResConverter));
		AddConverter(ResourceType("Model"),  CJING_NEW(ModelResConverter));
	}

	ResConverter::~ResConverter()
	{
		for (auto kvp : mResConverters) {
			CJING_SAFE_DELETE(kvp.second);
		}
		mResConverters.clear();
	}

	void ResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
		auto it = mResConverters.find(type.Type());
		if (it) {
			(*it)->OnEditorGUI(context, type, res);
		}
	}

	void ResConverter::AddConverter(const ResourceType& type, IResConverter* converter)
	{
		auto it = mResConverters.find(type.Type());
		if (it != nullptr) {
			return;
		}
		mResConverters.insert(type.Type(), converter);
	}

	bool ResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		auto it = mResConverters.find(type.Type());
		if (!it) {
			return false;
		}
		return (*it)->SupportsType(ext, type);
	}

	bool ResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		auto it = mResConverters.find(type.Type());
		if (!it) {
			return false;
		}
		return (*it)->Convert(context, type, src, dest);
	}

	LUMIX_PLUGIN_ENTRY(resConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(ResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}