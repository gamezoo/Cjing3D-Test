#pragma once

#include "includeDX11.h"
#include "gpu\resource.h"

namespace Cjing3D
{
namespace GPU
{
	struct ResourceDX11
	{
		ComPtr<ID3D11Resource> mResource;
		ComPtr<ID3D11ShaderResourceView>  mSRV;
		ComPtr<ID3D11UnorderedAccessView> mUAV;
		std::vector<ComPtr<ID3D11ShaderResourceView>> mSubresourceSRVs;
		std::vector<ComPtr<ID3D11UnorderedAccessView>> mSubresourceUAVS;

		I32 mNumSubResources = 0;
	};

	struct TextureDX11 : ResourceDX11
	{
		ComPtr<ID3D11RenderTargetView> mRTV;
		ComPtr<ID3D11DepthStencilView> mDSV;
		std::vector<ComPtr<ID3D11RenderTargetView>> mSubresourceRTVs;
		std::vector<ComPtr<ID3D11DepthStencilView>> mSubresourceDSVs;

		TextureDesc mDesc;
	};

	struct BufferDX11 : ResourceDX11
	{
		GPUBufferDesc mDesc;
	};

	struct SamplerStateDX11
	{
		ComPtr<ID3D11SamplerState> mHandle;
		SamplerDesc mDesc;
	};

	struct InputLayoutDX11
	{
		ComPtr<ID3D11InputLayout> mHandle;
	};

	struct ShaderDX11
	{
		U8* mByteCode = nullptr;
		U32 mByteCodeSize = 0;
		SHADERSTAGES mStage = SHADERSTAGES::SHADERSTAGES_COUNT;

		ComPtr<ID3D11VertexShader>  mVS;
		ComPtr<ID3D11PixelShader>   mPS;
		ComPtr<ID3D11ComputeShader> mCS;
		ComPtr<ID3D11HullShader>    mHS;
		ComPtr<ID3D11DomainShader>  mDS;
		ComPtr<ID3D11GeometryShader>mGS;
	};

	struct QueryDX11
	{
		ComPtr<ID3D11Query> mHandle;
	};

	struct PipelineStateDX11
	{
		PipelineStateDesc mDesc;

		ComPtr<ID3D11BlendState> mBS;
		ComPtr<ID3D11DepthStencilState> mDSS;
		ComPtr<ID3D11RasterizerState> mRS;
		ComPtr<ID3D11InputLayout> mIL;
	};
}
}