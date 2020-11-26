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

		Plugin* LoadPlugin(const char* path);
		bool ReloadPlugin(Plugin*& plugin);
		I32 ScanPlugins(const char* rootPath);
		Plugin* GetPlugin(const char* path);
		void GetPlugins(const StringID& pluginType, DynamicArray<Plugin*>& plugins);

		template<typename PluginT>
		void GetPlugins(DynamicArray<Plugin*>& plugins)
		{
			return GetPlugins(PluginT::Type(), plugins);
		}
	};
}