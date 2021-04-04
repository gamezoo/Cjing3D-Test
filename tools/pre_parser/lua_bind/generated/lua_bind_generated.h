// codegen_lua_bind
#pragma once

#include "luaHelper\luaBinder.h"
#include ".\test\geometry.h"
#include ".\test\intersectable.h"

using namespace Cjing3D;

void luabind_registers_AutoBindFunction(lua_State* l) {
    LuaBinder(l)
    .BeginClass<Sphere>("LuaSphere")
    .AddConstructor(_LUA_ARGS_())
    .AddConstructor(_LUA_ARGS_(F32x3 ,F32))
    .EndClass();

    LuaBinder(l)
    .BeginClass<AABB>("AABB")
    .AddFunction("CreateFromHalfWidth", &AABB::CreateFromHalfWidth)
    .AddMethod("GetMin", &AABB::GetMin)
    .AddMethod("GetMax", &AABB::GetMax)
    .EndClass();

    LuaBinder(l)
    .BeginClass<RectInt>("RectInt")
    .EndClass();

    LuaBinder(l)
    .BeginExtendClass<Rect, RectInt>("Rect")
    .AddMethod("GetPos", &Rect::GetPos)
    .AddMethod("GetSize", &Rect::GetSize)
    .EndClass();

}
