#ifndef SHADER_INTEROP_H
#define SHADER_INTEROP_H

#include "samplerMapping.h"

#ifdef CJING_SHADER_INTEROP

#include "math\maths.h"

using namespace Cjing3D;

typedef MATRIX matrix;
typedef F32x4x4 float4x4;
typedef F32x2 float2;
typedef F32x3 float3;
typedef F32x4 float4;
typedef I32x2 int2;
typedef I32x3 int3;
typedef I32x4 int4;
typedef U32   uint;
typedef U32x2 uint2;
typedef U32x3 uint3;
typedef U32x4 uint4;

#define CB_GETSLOT_NAME(name) __CBUFFER_SLOT_NAME_##name##__
#define CBUFFER(name, slot) static const int CB_GETSLOT_NAME(name) = slot; struct alignas(16) name
#define CONSTANTBUFFER(name, type, slot) static const int CB_GETSLOT_NAME(name) = slot; ConstantBuffer<type> name : register(b ## slot);

#else

#define CONSTANTBUFFER(name, type, slot) ConstantBuffer<type> name : register(b ## slot)


#endif
#endif
