#pragma once

#include "core\helper\stringID.h"

namespace Cjing3D
{
	class Plugin
	{
	public:
		const char* mName = nullptr;
		const char* mDesc = nullptr;
		const char* mFileName = nullptr;
		StringID    mType;
	};

	class StaticPlugin
	{
	public:
		using Creator = Plugin * (*)(const StringID& type);
		StaticPlugin(const char* name, Creator creator);

		static Plugin* GetPlugin(const char* name, const StringID& type);

		Creator mCreator = nullptr;
		const char* mName = nullptr;
		StaticPlugin* mNextPlugin = nullptr;
	};

#define PULGIN_DECLARE(NAME)                                                                \
	static StringID GetType() { return StringID(#NAME); }

#ifdef STATIC_PLUGINS							
#define LUMIX_PLUGIN_ENTRY(NAME)														    \
		extern "C" Plugin * GetPlugin_##NAME(const StringID & type);						\
		extern "C" { StaticPlugin static_plugin_##NAME(#NAME, GetPlugin_##NAME);}           \
		extern "C" Plugin * GetPlugin_##NAME(const StringID & type)
#else
#define LUMIX_PLUGIN_ENTRY(NAME)														   \
		extern "C" Plugin* GetPlugin(const StringID& type)
#endif

}