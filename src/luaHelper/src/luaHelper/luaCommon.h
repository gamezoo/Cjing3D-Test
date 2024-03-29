#pragma once

#include "lua\src\lua.hpp"

#include "debug.h"
#include "logger.h"

#ifdef _MSC_VER
#pragma warning( disable :4005 4018 4522 4715 4800 4996)
#endif

template<typename T>
struct ObjectIDGenerator
{
public:
	// 使用T类型下的ObjectIDGenerator结构体的static变量地址作为ID
	// 作为某个对象的位移标识符
	static void* GetID()
	{
		static bool id = false;
		return &id;
	}
};

using FunctionExportToLua = int(lua_State* l);