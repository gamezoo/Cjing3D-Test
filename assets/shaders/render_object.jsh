#ifndef RENDER_OBJECT_JSH
#define RENDER_OBJECT_JSH

#include "common/common.jsh"
#include "common/samplers.jsh"
#include "common/states.jsh"
#include "shaderInterop/shaderInteropRender.h"

/////////////////////////////////////////////////////////////////////////////////
struct ObjectMaterial
{
    MaterialCB gMaterial;
};

BindingSet MaterialBindings
{
    [register(SLOT_CB_MATERIAL)]
    ConstantBuffer<ObjectMaterial> constBuffer_Material;

    [register(SLOT_TEX_RENDERER_BASECOLORMAP)]
    Texture2D<float4> texture_BaseColorMap;

    [register(SLOT_TEX_RENDERER_NORMALMAP)]
    Texture2D<float3> texture_NormalMap;

    [register(SLOT_TEX_RENDERER_SURFACEMAP)]
    Texture2D<float4> texture_SurfaceMap;
};

/////////////////////////////////////////////////////////////////////////////////

struct VS_OBJECT_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

VS_OBJECT_OUTPUT VS_Object(uint id : SV_VertexID)
{
    VS_OBJECT_OUTPUT output;
	FullScreenTriangle(id, output.pos, output.uv);
    return output;
}

float4 PS_Object(VS_OBJECT_OUTPUT input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

#endif