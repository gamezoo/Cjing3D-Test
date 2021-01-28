#ifndef COMMON_BINDINGS_JSH
#define COMMON_BINDINGS_JSH

#include "shaderInterop/shaderInteropRender.h"

BindingSet FrameBindings
{
    [register(0)]
    ConstantBuffer<FrameCB> gFrameCB;

    [register(0)]
    Texture2D<unorm float4> gTexture;
};


#endif