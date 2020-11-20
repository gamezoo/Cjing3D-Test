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
			return true;
		}
	};

	LUMIX_PLUGIN_ENTRY(ResConverter)
	{
		return nullptr;
	}
}