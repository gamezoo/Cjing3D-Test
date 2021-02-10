#ifndef REGISTER_JSH
#define REGISTER_JSH

#include "metadata.jsh"

[internal("BlendStateRegister")]
struct BlendStateRegister
{
    BLENDMODE mBlendMode;
    Technique mTechnique;
};

[internal("RenderPassRegister")]
struct RenderPassRegister
{
    RENDERPASS mRenderPass;
    Technique mTechnique;

    BlendStateRegister mBlendState[8];
};

[internal("TechniqueRegister")]
struct TechniqueRegister
{
    RenderPassRegister mRenderPass[8];
};

#endif