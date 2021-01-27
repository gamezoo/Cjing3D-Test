#include "shaderImpl.h"

namespace Cjing3D
{
	void ShaderSerializer::SerializeRenderTargetBlend(const GPU::RenderTargetBlendStateDesc& desc, JsonArchive& archive)
	{
		archive.Write("mBlendEnable", desc.mBlendEnable);
		archive.Write("mSrcBlend", desc.mSrcBlend);
		archive.Write("mDstBlend", desc.mDstBlend);
		archive.Write("mBlendOp", desc.mBlendOp);
		archive.Write("mSrcBlendAlpha", desc.mSrcBlendAlpha);
		archive.Write("mDstBlendAlpha", desc.mDstBlendAlpha);
		archive.Write("mBlendOpAlpha", desc.mBlendOpAlpha);
		archive.Write("mRenderTargetWriteMask", desc.mRenderTargetWriteMask);
	}

	void ShaderSerializer::UnserializeRenderTargetBlend(GPU::RenderTargetBlendStateDesc& desc, JsonArchive& archive)
	{
		archive.Read("mBlendEnable", desc.mBlendEnable);
		archive.Read("mSrcBlend", desc.mSrcBlend);
		archive.Read("mDstBlend", desc.mDstBlend);
		archive.Read("mBlendOp", desc.mBlendOp);
		archive.Read("mSrcBlendAlpha", desc.mSrcBlendAlpha);
		archive.Read("mDstBlendAlpha", desc.mDstBlendAlpha);
		archive.Read("mBlendOpAlpha", desc.mBlendOpAlpha);
		archive.Read("mRenderTargetWriteMask", desc.mRenderTargetWriteMask);
	}

	void ShaderSerializer::SerializeDepthStencilOp(const GPU::DepthStencilOpDesc& desc, JsonArchive& archive)
	{
		archive.Write("mStencilFailOp", desc.mStencilFailOp);
		archive.Write("mStencilDepthFailOp", desc.mStencilDepthFailOp);
		archive.Write("mStencilPassOp", desc.mStencilPassOp);
		archive.Write("mStencilFunc", desc.mStencilFunc);
	}

	void ShaderSerializer::UnserializeDepthStencilOpe(GPU::DepthStencilOpDesc& desc, JsonArchive& archive)
	{
		archive.Read("mStencilFailOp", desc.mStencilFailOp);
		archive.Read("mStencilDepthFailOp", desc.mStencilDepthFailOp);
		archive.Read("mStencilPassOp", desc.mStencilPassOp);
		archive.Read("mStencilFunc", desc.mStencilFunc);
	}

	void ShaderSerializer::SerializeRenderState(const GPU::RenderStateDesc& desc, JsonArchive& archive)
	{
		// blendStateDesc
		auto& blendStateDesc = desc.mBlendState;
		archive.PushMap("mBlendState", [&](JsonArchive& archive) 
		{
			archive.Write("mAlphaToCoverageEnable", blendStateDesc.mAlphaToCoverageEnable);
			archive.Write("mIndependentBlendEnable", blendStateDesc.mIndependentBlendEnable);
			archive.PushMap("mRenderTarget", [&](JsonArchive& archive) {
				for (int i = 0; i < 8; i++)
				{
					archive.PushArray([&](JsonArchive& archive) {
						SerializeRenderTargetBlend(blendStateDesc.mRenderTarget[i], archive);
						});
				}
				});
		});

		// RasterizerStateDesc
		auto& rasterizerStateDesc = desc.mRasterizerState;
		archive.PushMap("mRasterizerState", [&](JsonArchive& archive)
		{
			archive.Write("mFillMode", rasterizerStateDesc.mFillMode);
			archive.Write("mCullMode", rasterizerStateDesc.mCullMode);
			archive.Write("mFrontCounterClockwise", rasterizerStateDesc.mFrontCounterClockwise);
			archive.Write("mDepthBias", rasterizerStateDesc.mDepthBias);
			archive.Write("mDepthBiasClamp", rasterizerStateDesc.mDepthBiasClamp);
			archive.Write("mSlopeScaleDepthBias", rasterizerStateDesc.mSlopeScaleDepthBias);
			archive.Write("mDepthClipEnable", rasterizerStateDesc.mDepthClipEnable);
			archive.Write("mMultisampleEnable", rasterizerStateDesc.mMultisampleEnable);
			archive.Write("mAntialiaseLineEnable", rasterizerStateDesc.mAntialiaseLineEnable);
			archive.Write("mConservativeRasterizationEnable", rasterizerStateDesc.mConservativeRasterizationEnable);
			archive.Write("mForcedSampleCount", rasterizerStateDesc.mForcedSampleCount);
		});

		// DepthStencilStateDesc
		auto& depthStencilStateDesc = desc.mDepthStencilState;
		archive.PushMap("mDepthStencilState", [&](JsonArchive& archive)
		{
			archive.Write("mDepthEnable", depthStencilStateDesc.mDepthEnable);
			archive.Write("mDepthWriteMask", depthStencilStateDesc.mDepthWriteMask);
			archive.Write("mDepthFunc", depthStencilStateDesc.mDepthFunc);
			archive.Write("mStencilEnable", depthStencilStateDesc.mStencilEnable);
			archive.Write("mStencilReadMask", depthStencilStateDesc.mStencilReadMask);
			archive.Write("mStencilWriteMask", depthStencilStateDesc.mStencilWriteMask);
			archive.PushMap("mFrontFace", [&](JsonArchive& archive) {
				SerializeDepthStencilOp(depthStencilStateDesc.mFrontFace, archive);
			});
			archive.PushMap("mBackFace", [&](JsonArchive& archive) {
				SerializeDepthStencilOp(depthStencilStateDesc.mBackFace, archive);
			});
		});
	}

	void ShaderSerializer::UnserializeRenderState(GPU::RenderStateDesc& desc, JsonArchive& archive)
	{
		// blendStateDesc
		auto& blendStateDesc = desc.mBlendState;
		archive.Read("mBlendState", [&](JsonArchive& archive)
		{
			archive.Read("mAlphaToCoverageEnable", blendStateDesc.mAlphaToCoverageEnable);
			archive.Read("mIndependentBlendEnable", blendStateDesc.mIndependentBlendEnable);
			archive.Read("mRenderTarget", [&](JsonArchive& archive) {
				for (int i = 0; i < 8; i++)
				{
					archive.Read(i, [&](JsonArchive& archive) {
						UnserializeRenderTargetBlend(blendStateDesc.mRenderTarget[i], archive);
					});
				}
			});
		});

		// RasterizerStateDesc
		auto& rasterizerStateDesc = desc.mRasterizerState;
		archive.Read("mRasterizerState", [&](JsonArchive& archive)
		{
			archive.Read("mFillMode", rasterizerStateDesc.mFillMode);
			archive.Read("mCullMode", rasterizerStateDesc.mCullMode);
			archive.Read("mFrontCounterClockwise", rasterizerStateDesc.mFrontCounterClockwise);
			archive.Read("mDepthBias", rasterizerStateDesc.mDepthBias);
			archive.Read("mDepthBiasClamp", rasterizerStateDesc.mDepthBiasClamp);
			archive.Read("mSlopeScaleDepthBias", rasterizerStateDesc.mSlopeScaleDepthBias);
			archive.Read("mDepthClipEnable", rasterizerStateDesc.mDepthClipEnable);
			archive.Read("mMultisampleEnable", rasterizerStateDesc.mMultisampleEnable);
			archive.Read("mAntialiaseLineEnable", rasterizerStateDesc.mAntialiaseLineEnable);
			archive.Read("mConservativeRasterizationEnable", rasterizerStateDesc.mConservativeRasterizationEnable);
			archive.Read("mForcedSampleCount", rasterizerStateDesc.mForcedSampleCount);
		});

		// DepthStencilStateDesc
		auto& depthStencilStateDesc = desc.mDepthStencilState;
		archive.Read("mDepthStencilState", [&](JsonArchive& archive)
		{
			archive.Read("mDepthEnable", depthStencilStateDesc.mDepthEnable);
			archive.Read("mDepthWriteMask", depthStencilStateDesc.mDepthWriteMask);
			archive.Read("mDepthFunc", depthStencilStateDesc.mDepthFunc);
			archive.Read("mStencilEnable", depthStencilStateDesc.mStencilEnable);
			archive.Read("mStencilReadMask", depthStencilStateDesc.mStencilReadMask);
			archive.Read("mStencilWriteMask", depthStencilStateDesc.mStencilWriteMask);
			archive.Read("mFrontFace", [&](JsonArchive& archive) {
				UnserializeDepthStencilOpe(depthStencilStateDesc.mFrontFace, archive);
			});
			archive.Read("mBackFace", [&](JsonArchive& archive) {
				UnserializeDepthStencilOpe(depthStencilStateDesc.mFrontFace, archive);
			});
		});
	}
}