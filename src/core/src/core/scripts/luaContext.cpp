#include "luaContext.h"
#include "core\engine.h"
#include "core\helper\debug.h"
#include "core\input\keyCode.h"
#include "core\helper\enumTraits.h"
#include "luabinder\luaBinder.h"

namespace Cjing3D {

LuaRef LuaContext::mSystemExports;

static const char* globalLuaString = R"(
	SystemExports = {}
)";

LuaContext::LuaContext(Engine& engine) :
	mEngine(engine)
{
}

LuaContext::~LuaContext()
{
}

void LuaContext::Initialize()
{
	Logger::Info("Initialize luaContext.");

	mLuaState = luaL_newstate();
	lua_atpanic(mLuaState, api_panic);
	luaL_openlibs(mLuaState);
	InitializeEnv(mLuaState);
}

void LuaContext::InitializeEnv(lua_State * l)
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

void LuaContext::InitializeEnum(lua_State * l)
{
	// bind input enum
	auto keyEnumBinder = LuaBinder(l).BeginModule("KeyCode");
	for (int index = 0; index < (int)(KeyCode::key_count); index++)
	{
		KeyCode keyCode = static_cast<KeyCode>(index);
		keyEnumBinder.AddEnum(std::string(EnumTraits::EnumToName(keyCode)), keyCode);
	}
}

void LuaContext::Uninitialize()
{
	mSystemExports.Clear();

	lua_close(mLuaState);
	mLuaState = nullptr;
}

bool LuaContext::DoLuaString(lua_State * l, const String & luaString, int arguments, int results)
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

bool LuaContext::DoLuaFile(lua_State* l, const String& path)
{
	if (LoadLuaFile(l, path))
	{
		if (lua_pcall(l, 0, 0, 0) != 0)
		{
			String errMsg = String("In ") + path + ": " + lua_tostring(l, -1) + "\n";
			luaL_traceback(l, l, NULL, 1);
			errMsg += lua_tostring(l, -1);
			Logger::Error(errMsg);
			lua_pop(l, 2);
			return false;
		}
		return true;
	}
	return false;
}

bool LuaContext::LoadLuaFile(lua_State * l, const String & path)
{
	String fileName(path);

	//auto filesystem = mEngine.GetFileSystem();

	//if ((fileName.find(".lua") == String::npos) ||
	//	!FileData::IsFileExists(fileName))
	//{
	//	fileName += ".lua";

	//	// 如果依旧不存在则返回
	//	if (!FileData::IsFileExists(fileName))
	//		return false;
	//}
	//const String buffer = FileData::ReadFile(fileName);
	//int result = luaL_loadbuffer(l, buffer.data(), buffer.size(), fileName.c_str());
	//if (result != 0)
	//{
	//	const String& errMsg = lua_tostring(l, -1);
	//	Logger::Warning(errMsg);
	//	lua_pop(l, 1);
	//	return false;
	//}
	return true;
}

int LuaContext::api_panic(lua_State*l)
{
	return 0;
}
}
