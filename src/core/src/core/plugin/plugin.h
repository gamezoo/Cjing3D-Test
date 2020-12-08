#pragma once

#include "core\helper\stringID.h"

namespace Cjing3D
{
	class Engine;

	class Plugin
	{
	public:
		const char* mName = nullptr;
		const char* mDesc = nullptr;
		const char* mFileName = nullptr;
		StringID    mType;

		virtual StringID GetType()const = 0;
	};

	class StaticPlugin
	{
	public:
		using Creator = Plugin * (*)(Engine& engine);
		StaticPlugin(const char* name, Creator creator);

		static Plugin* GetPlugin(const char* name, Engine& engine);

		Creator mCreator = nullptr;
		const char* mName = nullptr;
		StaticPlugin* mNextPlugin = nullptr;
	};

#define PULGIN_DECLARE(NAME)                                                                \
	    static StringID Type() { return StringID(#NAME); }                                  \
		virtual StringID GetType()const { return StringID(#NAME); }

#ifdef STATIC_PLUGINS							
#define LUMIX_PLUGIN_ENTRY(NAME)														    \
		extern "C" Plugin * GetPlugin_##NAME(Engine& engine);												\
		extern "C" { StaticPlugin static_plugin_##NAME(#NAME, GetPlugin_##NAME);}           \
		extern "C" Plugin * GetPlugin_##NAME(Engine& engine)
#else
#define LUMIX_PLUGIN_ENTRY(NAME)														   \
		extern "C" Plugin* GetPlugin(Engine& engine)
#endif

}