#include "pluginManager.h"

namespace Cjing3D
{
	void PluginManager::Initialize()
	{
	}

	void PluginManager::Uninitialize()
	{
	}

	bool PluginManager::IsInitialized()
	{
		return false;
	}

	Plugin* PluginManager::GetPlugin(const char* path)
	{
		return nullptr;
	}

	I32 PluginManager::GetPlugins(const StringID& pluginType, void* outPlugins, I32 maxPlugins)
	{
		return I32();
	}

}
