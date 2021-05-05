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

	struct SwapChainDX11
	{
		ComPtr<IDXGISwapChain1> mSwapChain;
		ComPtr<ID3D11Texture2D> mBackBuffer;
		ComPtr<ID3D11RenderTargetView> mRenderTargetView;
		I32 mBufferCount = 2;
	};

	struct TextureDX11 : ResourceDX11
	{
		ComPtr<ID3D11RenderTargetView> mRTV;
		ComPtr<ID3D11DepthStencilView> mDSV;
		std::vector<ComPtr<ID3D11RenderTargetView>> mSubresourceRTVs;
		std::vector<ComPtr<ID3D11DepthStencilView>> mSubresourceDSVs;
		bool mIsTransient = false;
		TextureDesc mDesc;
	};

	struct BufferDX11 : ResourceDX11
	{
		BufferDesc mDesc;
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

	struct BindingSRVDX11
	{
		SHADERSTAGES mStage = SHADERSTAGES::SHADERSTAGES_COUNT;
		I32 mSlot = 0;
		ID3D11ShaderResourceView* mSRV = nullptr;
	};

	struct BindingCBVDX11
	{
		SHADERSTAGES mStage = SHADERSTAGES::SHADERSTAGES_COUNT;
		I32 mSlot = 0;
		ID3D11Buffer* mBuffer = nullptr;
	};

	struct BindingUAVDX11
	{
		SHADERSTAGES mStage = SHADERSTAGES::SHADERSTAGES_COUNT;
		I32 mSlot = 0;
		ID3D11UnorderedAccessView* mUAV = nullptr;
	};

	struct BindingSamplerDX11
	{
		SHADERSTAGES mStage = SHADERSTAGES::SHADERSTAGES_COUNT;
		I32 mSlot = 0;
		ID3D11SamplerState* mSampler = nullptr;
	};

	struct PipelineBindingSetDX11
	{
		DynamicArray<BindingSRVDX11> mSRVs;
		DynamicArray<BindingCBVDX11> mCBVs;
		DynamicArray<BindingUAVDX11> mUAVs;
		DynamicArray<BindingSamplerDX11> mSAMs;
	};

	struct FrameBindingSetDX11
	{
		FrameBindingSetDesc mDesc;
	};

	class GraphicsDeviceDx11;

	struct GPUAllocatorDX11
	{
		static const U32 DefaultBufferSize = 1024 * 1024;
		BufferDesc mDesc;
		ResHandle mBuffer;
		U32 mOffset = 0;
		bool mIsDirty = false;
		I32 mResidentFrame = 0;
	};

	class CommandListDX11
	{
	public:
		CommandListDX11(GraphicsDeviceDx11& device);
		~CommandListDX11();

		ID3D11DeviceContext* GetContext();
		ID3D11CommandList* GetCommandList();
		ID3DUserDefinedAnnotation* GetAnnotation();

		void Reset();
		bool Submit(ID3D11DeviceContext& immediateContext);
		bool IsValid()const { return mDeviceContext.Get() != nullptr; }
		void ActivePipelineState(const PipelineStateDX11* pso);
		void RefreshPipelineState();
		GPUAllocation GPUAllocate(size_t size);
		void CommitAllactor();
		void ActiveFrameBindingSet(const FrameBindingSetDX11* bindingSet);
		const FrameBindingSetDX11* GetActiveFrameBindingSet() { return mActiveFrameBindingSet; }

	private:
		GraphicsDeviceDx11& mDevice;
		GPUAllocatorDX11 mAllocator;
		ComPtr<ID3D11DeviceContext> mDeviceContext;
		ComPtr<ID3D11CommandList> mCommandList;
		ComPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotations;

		GPUAllocatorDX11 mGPUAllocator;

		const FrameBindingSetDX11* mActiveFrameBindingSet = nullptr;
		const PipelineStateDX11* mActivePSO = nullptr;
		bool mIsPSODirty = true;
		
		// prev state info
		ID3D11VertexShader* mPrevVertexShader = nullptr;
		ID3D11PixelShader* mPrevPixelShader = nullptr;
		ID3D11ComputeShader* mPrevComputeShader = nullptr;
		ID3D11HullShader* mPrevHullShader = nullptr;
		ID3D11DomainShader* mPrevDomainShader = nullptr;
		ID3D11DepthStencilState* mPrevDepthStencilState = nullptr;
		ID3D11BlendState* mPrevBlendState = nullptr;
		ID3D11RasterizerState* mPrevRasterizerState = nullptr;
		ID3D11InputLayout* mPrevInputLayout = nullptr;
		PRIMITIVE_TOPOLOGY mPrevPrimitiveTopology = UNDEFINED_TOPOLOGY;
	};
}
}