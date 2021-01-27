#ifndef COMMON_BINDINGS_JSH
#define COMMON_BINDINGS_JSH

#include "shaderInterop/shaderInteropRender.h"

BindingSet FrameBindings
{
    [register(b0)]
    ConstantBuffer<FrameCB> gFrameCB;

    [register(t0)]
    Texture2D<unorm float4> gTexture;
};


#endif