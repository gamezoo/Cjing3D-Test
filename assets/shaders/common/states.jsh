#ifndef COMMON_STATES_JSH
#define COMMON_STATES_JSH

#include "common.jsh"

/////////////////////////////////////////////////////////////////////
// Rasteriazer States
/////////////////////////////////////////////////////////////////////
RasterizerState RAS_COMMON_FRONT = 
{
    .mFillMode = FILL_SOLID,
    .mCullMode = CULL_BACK,
    .mFrontCounterClockwise = 1,
    .mDepthBias = 0,
    .mDepthBiasClamp = 0,
    .mSlopeScaleDepthBias = 0,
    .mDepthClipEnable = 1,
    .mMultisampleEnable = 0,
    .mAntialiaseLineEnable = 0,
};

RasterizerState RAS_COMMON_BACK = 
{
    .mFillMode = FILL_SOLID,
    .mCullMode = CULL_FRONT,
    .mFrontCounterClockwise = 1,
    .mDepthBias = 0,
    .mDepthBiasClamp = 0,
    .mSlopeScaleDepthBias = 0,
    .mDepthClipEnable = 1,
    .mMultisampleEnable = 0,
    .mAntialiaseLineEnable = 0,
};

RasterizerState RAS_COMMON_SHADOW = 
{
    .mFillMode = FILL_SOLID,
    .mCullMode = CULL_BACK,
    .mFrontCounterClockwise = 1,
    .mDepthBias = -1,
    .mDepthBiasClamp = 0,
    .mSlopeScaleDepthBias = -4.0,
    .mDepthClipEnable = 1,
    .mMultisampleEnable = 0,
    .mAntialiaseLineEnable = 0,
};

RasterizerState RAS_COMMON_WIRE = 
{
    .mFillMode = FILL_WIREFRAME,
    .mCullMode = CULL_BACK,
    .mFrontCounterClockwise = 1,
    .mDepthBias = 0,
    .mDepthBiasClamp = 0,
    .mSlopeScaleDepthBias = 0,
    .mDepthClipEnable = 1,
    .mMultisampleEnable = 0,
    .mAntialiaseLineEnable = 0,
};

/////////////////////////////////////////////////////////////////////
// DepthStencil States
/////////////////////////////////////////////////////////////////////

DepthStencilState DSS_COMMON_DEFAULT = 
{
    .mDepthEnable = 1,
    .mDepthWriteMask = DEPTH_WRITE_MASK_ALL,
    .mDepthFunc = COMPARISON_GREATER,

    .mStencilEnable = 1,
    .mStencilReadMask = 0,
    .mStencilWriteMask = 0xFF,

    .mFrontFace = {
        .mStencilFailOp = STENCIL_OP_KEEP,
        .mStencilPassOp = STENCIL_OP_REPLACE,
        .mStencilDepthFailOp = STENCIL_OP_KEEP,
        .mStencilFunc = COMPARISON_ALWAYS,
    },
    .mBackFace = {
        .mStencilFailOp = STENCIL_OP_KEEP,
        .mStencilPassOp = STENCIL_OP_REPLACE,
        .mStencilDepthFailOp = STENCIL_OP_KEEP,
        .mStencilFunc = COMPARISON_ALWAYS,
    },
};

DepthStencilState DSS_COMMON_SHADOW = 
{
    .mDepthEnable = 1,
    .mDepthWriteMask = DEPTH_WRITE_MASK_ALL,
    .mDepthFunc = COMPARISON_GREATER,
    .mStencilEnable = 0,
};

DepthStencilState DSS_COMMON_DEPTHREAD = 
{
    .mDepthEnable = 1,
    .mDepthWriteMask = DEPTH_WRITE_MASK_ZERO,
    .mDepthFunc = COMPARISON_GREATER_EQUAL,
    .mStencilEnable = 0,
};

/////////////////////////////////////////////////////////////////////
// Blend States
/////////////////////////////////////////////////////////////////////

BlendState BS_COMMON_OPAQUE =
{
    .mAlphaToCoverageEnable = 0,
    .mIndependentBlendEnable = 0,
    .mRenderTarget[0] = {
        .mBlendEnable = 0,
        .mSrcBlend = BLEND_SRC_ALPHA,
        .mDstBlend = BLEND_INV_SRC_ALPHA,
        .mBlendOp = BLEND_OP_MAX,
        .mSrcBlendAlpha = BLEND_ONE,
        .mDstBlendAlpha = BLEND_ZERO,
        .mBlendOpAlpha = BLEND_OP_ADD,
        .mRenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL,
    },
};

BlendState BS_COMMON_TRANSPARENT =
{
    .mAlphaToCoverageEnable = 0,
    .mIndependentBlendEnable = 0,
    .mRenderTarget[0] = {
        .mBlendEnable = 1,
        .mSrcBlend = BLEND_SRC_ALPHA,
        .mDstBlend = BLEND_INV_SRC_ALPHA,
        .mBlendOp = BLEND_OP_ADD,
        .mSrcBlendAlpha = BLEND_ONE,
        .mDstBlendAlpha = BLEND_ONE,
        .mBlendOpAlpha = BLEND_OP_ADD,
        .mRenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL,
    },
};

BlendState BS_COMMON_PREMULTIPLIED =
{
    .mAlphaToCoverageEnable = 0,
    .mIndependentBlendEnable = 0,
    .mRenderTarget[0] = {
        .mBlendEnable = 1,
        .mSrcBlend = BLEND_ONE,
        .mDstBlend = BLEND_INV_SRC_ALPHA,
        .mBlendOp = BLEND_OP_ADD,
        .mSrcBlendAlpha = BLEND_ONE,
        .mDstBlendAlpha = BLEND_ONE,
        .mBlendOpAlpha = BLEND_OP_ADD,
        .mRenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL,
    },
};

#endif