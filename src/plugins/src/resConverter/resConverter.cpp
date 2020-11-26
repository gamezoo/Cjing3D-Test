#include "resConverter.h"

namespace Cjing3D
{
	class GeneralResConverter : public IResConverter
	{
	public:
		bool SupportsType(const char* ext, const ResourceType& type)override
		{
			return true;
		}
		
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override
		{
			return false;
		}
	};

	LUMIX_PLUGIN_ENTRY(resConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(GeneralResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}