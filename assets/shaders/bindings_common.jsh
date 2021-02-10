#ifndef COMMON_BINDINGS_JSH
#define COMMON_BINDINGS_JSH

#include "shaderInterop/shaderInteropRender.h"

BindingSet CommonBindings
{
    [register(SLOT_CB_FRAME)]
    ConstantBuffer<FrameCB> gFrameCB;

    [register(SLOT_CB_CAMERA)]
    ConstantBuffer<CameraCB> gCameraCB;
};


#endif