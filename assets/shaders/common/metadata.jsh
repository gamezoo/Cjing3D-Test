#ifndef METADATA_JSH
#define METADATA_JSH

[internal("BindingSet")]
declare_struct_type BindingSet;

[internal("RenderTargetBlendState")]
struct RenderTargetBlendState
{
    uint mBlendEnable;
    BlendType mSrcBlend;
    BlendType mDstBlend;
    BlendOp mBlendOp;
    BlendType mSrcBlendAlpha;
    BlendType mDstBlendAlpha;
    BlendOp mBlendOpAlpha;
    ColorWriteEnable mRenderTargetWriteMask;
};

[internal("BlendState")]
struct BlendState
{
    uint mAlphaToCoverageEnable;
    uint mIndependentBlendEnable;
    RenderTargetBlendState mRenderTarget[8];
};

[internal("DepthStencilFace")]
struct DepthStencilFace
{
    StencilOp mStencilFailOp;
    StencilOp mStencilDepthFailOp;
    StencilOp mStencilPassOp;
    ComparisonFunc mStencilFunc;
};

[internal("DepthStencilState")]
struct DepthStencilState
{
    uint mDepthEnable;
    DepthWriteMask mDepthWriteMask;
    ComparisonFunc mDepthFunc;
    uint mStencilEnable;
    uint mStencilReadMask;
    uint mStencilWriteMask;

    DepthStencilFace mFrontFace;
    DepthStencilFace mBackFace;
};

[internal("RasterizerState")]
struct RasterizerState
{
    FillMode mFillMode;
    CullMode mCullMode;
    uint mFrontCounterClockwise;
    uint mDepthBias;
    float mDepthBiasClamp;
    float mSlopeScaleDepthBias;
    uint mDepthClipEnable;
    uint mMultisampleEnable;
    uint mAntialiaseLineEnable;
    uint mConservativeRasterizationEnable;
    uint mForcedSampleCount;
};

[internal("SamplerState")]
struct SamplerState
{
    FILTER mFilter;
    TEXTURE_ADDRESS_MODE mAddressU;
    TEXTURE_ADDRESS_MODE mAddressV;
    TEXTURE_ADDRESS_MODE mAddressW;
    float mMipLODBias;
    uint mMaxAnisotropy;
    ComparisonFunc mComparisonFunc;
    float mMinLOD;
    float mMaxLOD;
};

[internal("RenderState")]
struct RenderState
{
    BlendState mBlendState;
    DepthStencilState mDepthStencilState;
    RasterizerState mRasterizerState;
};

[internal("Technique")]
struct Technique
{
    function mVertexShader;
    function mGeometryShader;
    function mHullShader;
    function mDomainShader;
    function mPixelShader;
    function mComputeShader;

    RenderState mRenderState;
};

#endif
