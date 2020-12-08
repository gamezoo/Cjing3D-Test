#pragma once

#include "luabinder\luaRef.h"
#include "luabinder\luaTools.h"
#include "core\common\common.h"

namespace Cjing3D
{

class Engine;

class LuaContext
{
public:
	LuaContext(Engine& engine);
	~LuaContext();

	void Initialize();
	void Uninitialize();

	lua_State* GetLuaState() { return mLuaState; }

	using FunctionExportToLua = int(lua_State* l);
	static FunctionExportToLua
		api_panic;

	static bool DoLuaString(lua_State*l, const String& luaString, int arguments = 0, int results = 0);
	static bool DoLuaFile(lua_State*l, const String& path);
	static bool LoadLuaFile(lua_State*l, const String& path);

	// system lua ref
	static LuaRef mSystemExports;
	static LuaRef mSystemEnumRef;

private:
	void InitializeEnv(lua_State* l);
	void InitializeEnum(lua_State* l);

	Engine& mEngine;
	lua_State* mLuaState = nullptr;
};
}