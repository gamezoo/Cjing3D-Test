#pragma once

#include "maths_common.h"

namespace Cjing3D
{
namespace Random
{
	U32   GetRandomInt(U32 x);
	U32   GetRandomInt(U32 x, U32 y);
	float GetRandomFloat(F32 x = 1.0f);
	F32   GetRandomFloat(F32 x, F32 y);
	F64   GetRandomDouble(F64 x, F64 y);
	F64   GetRandomDouble(F64 x = 1.0);
}
}