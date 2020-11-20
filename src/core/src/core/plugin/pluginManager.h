#pragma once

#include "plugin.h"
#include "core\container\dynamicArray.h"
#include "core\helper\stringID.h"
#include "core\common\definitions.h"

namespace Cjing3D
{
	namespace PluginManager
	{
		void Initialize();
		void Uninitialize();
		bool IsInitialized();

		Plugin* GetPlugin(const char* path);

		I32 GetPlugins(const StringID& pluginType, void* outPlugins, I32 maxPlugins);

		template<typename PluginT>
		I32 GetPlugins(PluginT* outPlugins, I32 maxPlugins)
		{
			return GetPlugins(PluginT::GetType(), outPlugins, maxPlugins);
		}
	};
}