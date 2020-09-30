// codegen_lua_bind
#pragma once

#include "luabinder\luaBinder.h"
#include ".\test\geometry.h"

using namespace Cjing3D;

void luabind_registers_AutoBindFunction(lua_State* l) {
    LuaBinder(l)
    .BeginClass<RectInt>("RectInt")
    .EndClass();

    LuaBinder(l)
    .BeginExtendClass<Rect, RectInt>("Rect")
    .AddMethod("GetPos", &Rect::GetPos)
    .AddMethod("GetSize", &Rect::GetSize)
    .EndClass();

}
