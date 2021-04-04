#pragma once

#include "luaHelper\luaRef.h"
#include "luaHelper\luaTools.h"
#include "core\common\common.h"
#include "core\scene\universe.h"
#include "core\plugin\modulePulgin.h"

namespace Cjing3D
{
	class Engine;
	class LuaScriptSystem;

	class LuaScriptScene : public ECS::IScene
	{
	public:
		explicit LuaScriptScene(LuaScriptSystem& context, Universe& universe);
		virtual ~LuaScriptScene();

		void Initialize()override;
		void Uninitialize()override;
		void Update(F32 dt)override;
		void LateUpdate(F32 dt)override;
		void Clear()override;

		Universe& GetUniverse()override { return mUniverse; }
		lua_State* GetLuaState() { return mLuaState; }

		using FunctionExportToLua = int(lua_State* l);
		static FunctionExportToLua
			api_panic;

		bool DoLuaString(lua_State* l, const String& luaString, int arguments = 0, int results = 0);

		// system lua ref
		static LuaRef mSystemExports;
		static LuaRef mSystemEnumRef;

	private:
		void InitializeEnv(lua_State* l);
		void InitializeEnum(lua_State* l);

		LuaScriptSystem& mContext;
		Engine& mEngine;
		Universe& mUniverse;
		lua_State* mLuaState = nullptr;
	};

	class LuaScriptSystem : public ModulerPlugin
	{
	public:
		explicit LuaScriptSystem(Engine& engine) :
			mEngine(engine)
		{}
		virtual ~LuaScriptSystem() {}

		void Initialize()override {}
		void Uninitialize()override {}
		void Update(F32 dt)override {}
		void CreateScene(Universe& universe)override;
		
		Engine& GetEngine() {
			return mEngine;
		}

	private:
		Engine& mEngine;
	};

}