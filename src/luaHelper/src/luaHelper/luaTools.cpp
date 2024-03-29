#include "luaTools.h"
#include "luaRef.h"
#include "logger.h"
#include "debug.h"

#include<iostream>
#include<sstream>

namespace Cjing3D
{
	namespace LuaTools
	{
		void CheckType(lua_State * l, int index, int exceptedType)
		{
			if (lua_type(l, index) != exceptedType)
			{
				LuaException::ArgError(l, index, std::string("excepted") + std::string(luaL_typename(l, exceptedType)) +
					" but get" + luaL_typename(l, index));
			}
		}

		int GetPositiveIndex(lua_State * l, int index)
		{
			int positiveIndex = index;
			if (index < 0 && index >= -lua_gettop(l))
				positiveIndex = lua_gettop(l) + index + 1;

			return positiveIndex;
		}

		void PrintLuaStack(lua_State * l)
		{
			int stackSize = lua_gettop(l);
			std::ostringstream oss;
			oss << std::endl;
			for (int i = 1; i <= stackSize; i++)
			{
				switch (lua_type(l, i))
				{
				case LUA_TSTRING:
					oss << "\"" << lua_tostring(l, i) << "\"";
					break;
				case LUA_TBOOLEAN:
					oss << (lua_toboolean(l, i) ? "True" : "False");
					break;
				case LUA_TNUMBER:
					oss << (lua_tonumber(l, i));
					break;
				default:
					oss << lua_typename(l, lua_type(l, i));
					break;
				}
				oss << std::endl;
			}
			LuaTools::Logger::Debug(oss.str());
		}

		void PrintInIndex(lua_State * l, int index)
		{
			index = LuaTools::GetPositiveIndex(l, index);
			std::ostringstream oss;
			switch (lua_type(l, index))
			{
			case LUA_TSTRING:
				oss << "\"" << lua_tostring(l, index) << "\"";
				break;
			case LUA_TBOOLEAN:
				oss << (lua_toboolean(l, index) ? "True" : "False");
				break;
			case LUA_TNUMBER:
				oss << (lua_tonumber(l, index));
				break;
			default:
				oss << lua_typename(l, lua_type(l, index));
				break;
			}
			LuaTools::Logger::Debug(oss.str());
		}

		int GetCallDepth(lua_State * l)
		{
			int depth = 0;
			lua_Debug info;
			for (; lua_getstack(l, depth + 1, &info) != 0; depth++);
			return depth;
		}

		void CheckAssertion(lua_State*l, bool assertion, const std::string & msg)
		{
			if (assertion == false) {
				LuaException::Error(l, msg);
			}
		}

		int GetArgCount(lua_State * l)
		{
			return lua_gettop(l);
		}

		bool CallFunction(lua_State * l, int arguments, int results, const std::string & functionName)
		{
			if (lua_pcall(l, arguments, results, 0) != 0)
			{
				std::string errMsg = std::string("In ") + functionName + ": " + lua_tostring(l, -1) + "\n";
				luaL_traceback(l, l, NULL, 1);
				errMsg += lua_tostring(l, -1);
				LuaTools::Debug::Error(errMsg);
				lua_pop(l, 2);
				return false;
			}

			return true;
		}
	}
}