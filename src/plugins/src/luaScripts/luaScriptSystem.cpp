#include "luaScriptSystem.h"
#include "core\engine.h"
#include "core\helper\debug.h"
#include "core\input\keyCode.h"
#include "core\helper\enumTraits.h"
#include "luaHelper\luaBinder.h"

namespace Cjing3D {

LuaRef LuaScriptScene::mSystemExports;

static const char* globalLuaString = R"(
	SystemExports = {}
)";

LuaScriptScene::LuaScriptScene(LuaScriptSystem& context, Universe& universe) :
	mContext(context),
	mEngine(context.GetEngine()),
	mUniverse(universe)
{
}

LuaScriptScene::~LuaScriptScene()
{
}

void LuaScriptScene::Initialize()
{
	Logger::Info("Initialize luaContext.");

	mLuaState = luaL_newstate();
	lua_atpanic(mLuaState, api_panic);
	luaL_openlibs(mLuaState);
	InitializeEnv(mLuaState);
}

bool LuaScriptScene::DoLuaString(lua_State* l, const String& luaString, int arguments, int results)
{
	if (luaL_loadstring(l, luaString.c_str()) == 0) {
		if (arguments > 0) {
			// stack: arg1 arg2 func
			lua_insert(l, -(arguments + 1));
			// func stack: arg1 arg2
		}
		return LuaTools::CallFunction(l, arguments, results, "Load Lua String.");
	}
	return false;
}

void LuaScriptScene::InitializeEnv(lua_State * l)
{
	if (!DoLuaString(l, globalLuaString))
	{
		Logger::Error("Failed to load lua initialzed scripts.");
		return;
	}

	lua_getglobal(l, "SystemExports");
	Debug::CheckAssertion(!lua_isnil(l, 1), "Lua env initialized failed.");
	mSystemExports = LuaRef::CreateRef(l);

	InitializeEnum(l);
}

void LuaScriptScene::InitializeEnum(lua_State * l)
{
	// bind input enum
	auto keyEnumBinder = LuaBinder(l).BeginModule("KeyCode");
	for (int index = 0; index < (int)(KeyCode::key_count); index++)
	{
		KeyCode keyCode = static_cast<KeyCode>(index);
		keyEnumBinder.AddEnum(std::string(EnumTraits::EnumToName(keyCode)), keyCode);
	}
}

void LuaScriptScene::Uninitialize()
{
	mSystemExports.Clear();

	lua_close(mLuaState);
	mLuaState = nullptr;
}

void LuaScriptScene::Update(F32 dt)
{
}

void LuaScriptScene::LateUpdate(F32 dt)
{
}

void LuaScriptScene::Clear()
{
}

int LuaScriptScene::api_panic(lua_State*l)
{
	return 0;
}

void LuaScriptSystem::CreateScene(Universe& universe)
{
	UniquePtr<LuaScriptScene> scene = CJING_MAKE_UNIQUE<LuaScriptScene>(*this, universe);
	universe.AddScene(std::move(scene));
}

LUMIX_PLUGIN_ENTRY(luaScripts)
{
	return CJING_NEW(LuaScriptSystem)(engine);
}

}
