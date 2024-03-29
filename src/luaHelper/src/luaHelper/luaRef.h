#pragma once

#include "luaCommon.h"
#include "luaTools.h"
#include "luaArg.h"

#include <string>

namespace Cjing3D
{
	class LuaRef
	{
	public:
		LuaRef();
		LuaRef(lua_State*l, int ref);
		LuaRef(const LuaRef& other);
		LuaRef(LuaRef&& other);
		LuaRef& operator=(const LuaRef&other);
		LuaRef& operator=(LuaRef&& other);
		~LuaRef();

		bool operator ==(const LuaRef& ref)const;
		bool operator !=(const LuaRef& ref)const;

		static LuaRef CreateRef(lua_State*l);
		static LuaRef CreateRef(lua_State*l, int index);
		static LuaRef CreateTable(lua_State* l, int narray = 0, int nrec = 0);
		static LuaRef CreateGlobalRef(lua_State* l);
		static LuaRef CreateRefFromPtr(lua_State*l, void* ptr);

		template<typename T>
		static LuaRef CreateRefFromValue(lua_State*l, const T& value)
		{
			LuaTools::Push(l, value);
			return CreateRef(l);
		}

		template<typename... Args>
		static LuaRef CreateFunctionWithArgs(lua_State*l, lua_CFunction func, Args&&... args)
		{
			LuaPushArgs<Args...>::Push(l, std::forward<Args>(args)...);
			//LuaTools::PushArgs(l, std::forward<Args>(args)...);
			lua_pushcclosure(l, func, sizeof...(args));
			return CreateRef(l);
		}

		template<typename T>
		static LuaRef CreateFuncWithUserdata(lua_State*l, lua_CFunction func, const T& userdata)
		{
			LuaTools::BindingUserData::PushUserdata<T>(l, userdata);
			lua_pushcclosure(l, func, 1);
			return CreateRef(l);
		}

		template<typename T>
		static std::enable_if_t<std::is_function<T>::value, LuaRef>
			CreateFunc(lua_State*l, lua_CFunction func, const T& userdata)
		{
			// 将函数指针作为userdata压栈
			LuaTools::BindingUserData::PushUserdata<T*>(l, &userdata);
			lua_pushcclosure(l, func, 1);
			return CreateRef(l);
		}

		template<typename T>
		static std::enable_if_t<!std::is_function<T>::value, LuaRef>
			CreateFunc(lua_State*l, lua_CFunction func, const T& userdata)
		{
			// 将函数指针作为userdata压栈
			LuaTools::BindingUserData::PushUserdata<T>(l, userdata);
			lua_pushcclosure(l, func, 1);
			return CreateRef(l);
		}

		template<typename T>
		static LuaRef CreateFuncWithPtr(lua_State*l, lua_CFunction func, T* p)
		{
			lua_pushlightuserdata(l, p);
			lua_pushcclosure(l, func, 1);
			return CreateRef(l);
		}

		bool IsEmpty()const;
		bool IsRefEmpty()const;
		int  GetRef()const;
		void Push()const;
		void Clear();
		bool IsFunction()const;
		lua_State* GetLuaState()const;

		template<typename V, typename K>
		void RawSet(const K& key, const V& value)
		{
			Push();
			LuaTools::Push(l, key);
			LuaTools::Push(l, value);
			lua_rawset(l, -3);
			lua_pop(l, 1);
		}

		template<typename V>
		void RawSetp(void* key, const V& value)
		{
			Push();
			LuaTools::Push(l, value);
			lua_rawsetp(l, -2, key);
			lua_pop(l, 1);
		}

		template<typename V = LuaRef>
		V RawGetp(void* key)
		{
			Push();
			lua_rawgetp(l, -1, key);
			V v = LuaTools::Get<V>(l, -1);
			lua_pop(l, 2);
			return v;
		}

		template<typename V = LuaRef, typename K>
		V RawGet(const K& key)
		{
			Push();
			LuaTools::Push(l, key);
			lua_rawget(l, -2);
			V v = LuaTools::Get<V>(l, -1);
			lua_pop(l, 2);
			return v;
		}

		void SetMetatable(LuaRef& luaRef);

		static LuaRef NULL_REF;
	private:
		lua_State * l;
		int mRef;

	};

	// see LuaTypeMapping.h
	// define LuaTypeNormalMapping to support normal type mapping lua
	template<>
	struct LuaTypeNormalMapping<LuaRef>
	{
		static void Push(lua_State*l, const LuaRef& value)
		{
			if (value.IsEmpty())
			{
				lua_pushnil(l);
			}
			else
			{
				value.Push();
			}
		}

		static LuaRef Get(lua_State*l, int index)
		{
			if (lua_isnil(l, index))
			{
				return LuaRef::NULL_REF;
			}
			else
			{
				return LuaRef::CreateRef(l, index);
			}
		}

		static LuaRef Opt(lua_State* l, int index, const LuaRef& defValue)
		{
			return lua_isnone(l, index) ? defValue : Get(l, index);
		}
	};
}