#include "plugin.h"
#include "pluginManager.h"
#include "core\string\string.h"
#include "core\engine.h"

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

	Plugin* StaticPlugin::GetPlugin(const char* name, Engine& engine)
	{
		StaticPlugin* node = staticPluginRoot;
		while (node)
		{
			if (EqualString(name, node->mName)) {
				return node->mCreator(engine);
			}
			node = node->mNextPlugin;
		}
		return nullptr;
	}
}