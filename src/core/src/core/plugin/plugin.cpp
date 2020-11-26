#include "plugin.h"
#include "pluginManager.h"
#include "core\string\string.h"

namespace Cjing3D
{
	static StaticPlugin* staticPluginRoot = nullptr;

	StaticPlugin::StaticPlugin(const char* name, Creator creator) :
		mName(name),
		mCreator(creator)
	{
		mNextPlugin = staticPluginRoot;
		staticPluginRoot = this;
	}

	Plugin* StaticPlugin::GetPlugin(const char* name)
	{
		StaticPlugin* node = staticPluginRoot;
		while (node)
		{
			if (EqualString(name, node->mName)) {
				return node->mCreator();
			}
			node = node->mNextPlugin;
		}
		return nullptr;
	}
}