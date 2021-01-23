#include "renderImage.h"
#include "renderer.h"

namespace Cjing3D
{
	bool mIsInitialized = false;
	GPU::BlendStateDesc mBlendState[BLENDMODE_COUNT];
	GPU::RasterizerStateDesc mRasterizerState;
	GPU::DepthStencilStateDesc mDepthStencilState;

	/// ///////////////////////////////////////////////////////////////////
	// Fullscreen pipeline
	void RenderImage::Initialize()
	{
		// initialize rasterizerState
		GPU::RasterizerStateDesc rs;
		rs.mFillMode = GPU::FILL_SOLID;
		rs.mCullMode = GPU::CULL_NONE;
		rs.mFrontCounterClockwise = false;
		rs.mDepthBias = 0;
		rs.mDepthBiasClamp = 0;
		rs.mSlopeScaleDepthBias = 0;
		rs.mDepthClipEnable = true;
		rs.mMultisampleEnable = false;
		rs.mAntialiaseLineEnable = false;
		mRasterizerState = rs;

		// initialize depthStecncilState 
		GPU::DepthStencilStateDesc dsd;
		dsd.mDepthEnable = false;
		dsd.mStencilEnable = false;
		mDepthStencilState = dsd;

		////////////////////////////////////////////////////////////////////
		// initialize blend state
		GPU::BlendStateDesc bd;
		bd.mRenderTarget[0].mBlendEnable = true;
		bd.mRenderTarget[0].mSrcBlend = GPU::BLEND_SRC_ALPHA;
		bd.mRenderTarget[0].mDstBlend = GPU::BLEND_INV_SRC_ALPHA;
		bd.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
		bd.mRenderTarget[0].mSrcBlendAlpha = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mDstBlendAlpha = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
		bd.mRenderTarget[0].mRenderTargetWriteMask = GPU::COLOR_WRITE_ENABLE_ALL;
		mBlendState[BLENDMODE_ALPHA] = bd;

		bd.mRenderTarget[0].mBlendEnable = true;
		bd.mRenderTarget[0].mSrcBlend = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mDstBlend = GPU::BLEND_INV_SRC_ALPHA;
		bd.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
		bd.mRenderTarget[0].mSrcBlendAlpha = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mDstBlendAlpha = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
		bd.mRenderTarget[0].mRenderTargetWriteMask = GPU::COLOR_WRITE_ENABLE_ALL;
		mBlendState[BLENDMODE_PREMULTIPLIED] = bd;

		bd.mRenderTarget[0].mBlendEnable = false;
		bd.mRenderTarget[0].mRenderTargetWriteMask = GPU::COLOR_WRITE_ENABLE_ALL;
		mBlendState[BLENDMODE_OPAQUE] = bd;

		bd.mRenderTarget[0].mBlendEnable = true;
		bd.mRenderTarget[0].mSrcBlend = GPU::BLEND_SRC_ALPHA;
		bd.mRenderTarget[0].mDstBlend = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
		bd.mRenderTarget[0].mSrcBlendAlpha = GPU::BLEND_ZERO;
		bd.mRenderTarget[0].mDstBlendAlpha = GPU::BLEND_ONE;
		bd.mRenderTarget[0].mBlendOp = GPU::BLEND_OP_ADD;
		bd.mRenderTarget[0].mRenderTargetWriteMask = GPU::COLOR_WRITE_ENABLE_ALL;
		mBlendState[BLENDMODE_ADDITIVE] = bd;

		mIsInitialized = true;
	}

	void RenderImage::Draw(const GPU::ResHandle& tex, const ImageParams& param, GPU::CommandList& cmd)
	{
		if (!mIsInitialized) {
			return;
		}

		auto shader = Renderer::GetShader(SHADERTYPE_IMAGE);
		if (!shader) {
			return;
		}

		// fullscreen
		if (param.IsFullScreen())
		{
			ShaderTechniqueDesc desc = {};
			desc.mBlendState = &mBlendState[param.mBlendFlag];
			desc.mDepthStencilState = &mDepthStencilState;
			desc.mRasterizerState = &mRasterizerState;
			desc.mPrimitiveTopology = GPU::TRIANGLESTRIP;

			auto tech = shader->CreateTechnique("TECH_FULLSCREEN", desc);
			auto pipelineState = tech.GetPipelineState();
			cmd.BindPipelineState(pipelineState);
			cmd.Draw(3, 0);
		}
	}
}