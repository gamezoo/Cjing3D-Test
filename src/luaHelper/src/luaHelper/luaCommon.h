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
	// ʹ��T�����µ�ObjectIDGenerator�ṹ���static������ַ��ΪID
	// ��Ϊĳ�������λ�Ʊ�ʶ��
	static void* GetID()
	{
		static bool id = false;
		return &id;
	}
};

using FunctionExportToLua = int(lua_State* l);