#include "resourceDX11.h"
#include "deviceDX11.h"
#include "gpu\commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
namespace GPU
{
	CommandListDX11::CommandListDX11(GraphicsDeviceDx11& device) :
		mDevice(device)
	{
		ID3D11Device& deviceDX11 = device.GetDevice();
		HRESULT hr = deviceDX11.CreateDeferredContext(0, &mDeviceContext);
		if (FAILED(hr))
		{
			Logger::Error("[CommandList] Failed to create deferred context: %08X", hr);
			return;
		}

		hr = mDeviceContext.As(&mUserDefinedAnnotations);
		if (FAILED(hr))
		{
			Logger::Error("[CommandList] Failed to set user defined annotations: %08X", hr);
			return;
		}

		// create gpu allocator buffer
		BufferDesc desc = {};
		desc.mByteWidth = GPUAllocatorDX11::DefaultBufferSize;
		desc.mBindFlags = BIND_SHADER_RESOURCE | BIND_INDEX_BUFFER | BIND_VERTEX_BUFFER;
		desc.mUsage = USAGE_DYNAMIC;
		desc.mCPUAccessFlags = CPU_ACCESS_WRITE;
		desc.mMiscFlags = RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

		mAllocator.mDesc = desc;
		mAllocator.mBuffer = GPU::AllocateHandle(RESOURCETYPE_BUFFER);
		if (!device.CreateBuffer(mAllocator.mBuffer, &desc, nullptr))
		{
			GPU::DestroyResource(mAllocator.mBuffer);
			Logger::Error("[CommandList] Failed to create buffer.");
			return;
		}
		device.SetResourceName(mAllocator.mBuffer, "CommandListFrameAllocator");
	}

	CommandListDX11::~CommandListDX11()
	{
		if (mAllocator.mBuffer != ResHandle::INVALID_HANDLE) {
			GPU::DestroyResource(mAllocator.mBuffer);
		}
	}

	ID3D11DeviceContext* CommandListDX11::GetContext()
	{
		return mDeviceContext.Get();
	}

	ID3D11CommandList* CommandListDX11::GetCommandList()
	{
		return mCommandList.Get();
	}

	ID3DUserDefinedAnnotation* CommandListDX11::GetAnnotation()
	{
		return mUserDefinedAnnotations.Get();
	}

	bool CommandListDX11::Submit(ID3D11DeviceContext& immediateContext)
	{
		if (FAILED(mDeviceContext->FinishCommandList(false, &mCommandList))) {
			return false;
		}
		immediateContext.ExecuteCommandList(mCommandList.Get(), false);
		mCommandList.Reset();
		return true;
	}

	void CommandListDX11::ActivePipelineState(const PipelineStateDX11* pso)
	{
		if (mActivePSO == pso) {
			return;
		}

		mActivePSO = pso;
		mIsPSODirty = true;
	}

	void CommandListDX11::Reset()
	{
		mPrevVertexShader = nullptr;
		mPrevPixelShader = nullptr;
		mPrevComputeShader = nullptr;
		mPrevHullShader = nullptr;
		mPrevDomainShader = nullptr;
		mPrevInputLayout = nullptr;
		mPrevRasterizerState = nullptr;
		mPrevDepthStencilState = nullptr;
		mPrevBlendState = nullptr;
		mPrevPrimitiveTopology = UNDEFINED_TOPOLOGY;

		// reset active obj
		mActiveFrameBindingSet = nullptr;
		mActivePSO = nullptr;
		mIsPSODirty = false;

		// reset viewport
		D3D11_VIEWPORT vp = {};
		vp.Width    = (F32)mDevice.mResolution.x();
		vp.Height   = (F32)mDevice.mResolution.y();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		mDeviceContext->RSSetViewports(1, &vp);

		// reset scissor rect
		D3D11_RECT rect = {};
		rect.left   = INT32_MIN;
		rect.top    = INT32_MIN;
		rect.right  = INT32_MAX;
		rect.bottom = INT32_MAX;
		mDeviceContext->RSSetScissorRects(1, &rect);

		// bind static samplers
		for (I32 shaderStage = 0; shaderStage < SHADERSTAGES_COUNT; shaderStage++)
		{
			for (const auto& sampler : mDevice.mStaticSamplers)
			{
				ID3D11SamplerState* samplerState = sampler.mSampler != ResHandle::INVALID_HANDLE ?
					mDevice.mSamplers.Read(sampler.mSampler)->mHandle.Get() : nullptr;
				if (samplerState != nullptr)
				{
					SHADERSTAGES stage = SHADERSTAGES(shaderStage);
					switch (stage)
					{
					case SHADERSTAGES_VS:
						mDeviceContext->VSSetSamplers(sampler.mSlot, 1, &samplerState);
						break;
					case SHADERSTAGES_GS:
						mDeviceContext->GSSetSamplers(sampler.mSlot, 1, &samplerState);
						break;
					case SHADERSTAGES_HS:
						mDeviceContext->HSSetSamplers(sampler.mSlot, 1, &samplerState);
						break;
					case SHADERSTAGES_DS:
						mDeviceContext->DSSetSamplers(sampler.mSlot, 1, &samplerState);
						break;
					case SHADERSTAGES_PS:
						mDeviceContext->PSSetSamplers(sampler.mSlot, 1, &samplerState);
						break;
					case SHADERSTAGES_CS:
						mDeviceContext->CSSetSamplers(sampler.mSlot, 1, &samplerState);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	void CommandListDX11::RefreshPipelineState()
	{
		if (!mIsPSODirty) {
			return;
		}

		PipelineStateDesc desc = mActivePSO->mDesc;

		ID3D11VertexShader* vs = desc.mVS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mVS)->mVS.Get() : nullptr;
		if (vs != mPrevVertexShader)
		{
			mDeviceContext->VSSetShader(vs, nullptr, 0);
			mPrevVertexShader = vs;
		}
		ID3D11PixelShader* ps = desc.mPS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mPS)->mPS.Get() : nullptr;
		if (ps != mPrevPixelShader)
		{
			mDeviceContext->PSSetShader(ps, nullptr, 0);
			mPrevPixelShader = ps;
		}
		ID3D11HullShader* hs = desc.mHS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mHS)->mHS.Get() : nullptr;
		if (hs != mPrevHullShader)
		{
			mDeviceContext->HSSetShader(hs, nullptr, 0);
			mPrevHullShader = hs;
		}
		ID3D11DomainShader* ds = desc.mDS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mDS)->mDS.Get() : nullptr;
		if (ds != mPrevDomainShader)
		{
			mDeviceContext->DSSetShader(ds, nullptr, 0);
			mPrevDomainShader = ds;
		}
		ID3D11BlendState* bs = mActivePSO->mBS.Get();
		if (bs != mPrevBlendState)
		{
			const float factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			mDeviceContext->OMSetBlendState(bs, factor, 0xFFFFFFFF);
			mPrevBlendState = bs;
		}
		ID3D11RasterizerState* rs = mActivePSO->mRS.Get();
		if (rs != mPrevRasterizerState)
		{
			mDeviceContext->RSSetState(rs);
			mPrevRasterizerState = rs;
		}
		ID3D11DepthStencilState* dss = mActivePSO->mDSS.Get();
		if (dss != mPrevDepthStencilState)
		{
			mDeviceContext->OMSetDepthStencilState(dss, 0);
			mPrevDepthStencilState = dss;
		}
		ID3D11InputLayout* il = mActivePSO->mIL.Get();
		if (il != mPrevInputLayout)
		{
			mDeviceContext->IASetInputLayout(il);
			mPrevInputLayout = il;
		}
		if (desc.mPrimitiveTopology != mPrevPrimitiveTopology)
		{
			D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			switch (desc.mPrimitiveTopology)
			{
			case TRIANGLELIST:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				break;
			case TRIANGLESTRIP:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
				break;
			case POINTLIST:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
				break;
			case LINELIST:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
				break;
			case PATCHLIST_3:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
				break;
			case PATCHLIST_4:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
				break;
			default:
				primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
				break;
			}

			mDeviceContext->IASetPrimitiveTopology(primitiveTopology);
			mPrevPrimitiveTopology = desc.mPrimitiveTopology;
		}
	}

	GPUAllocation CommandListDX11::GPUAllocate(size_t size)
	{
		GPUAllocation allocation;
		if (size == 0) {
			return allocation;
		}

		if (mAllocator.mDesc.mByteWidth <= size)
		{
			if (mAllocator.mBuffer != ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(mAllocator.mBuffer);
			}

			// grow the allocator
			mAllocator.mDesc.mByteWidth *= 2;
			mAllocator.mBuffer = GPU::AllocateHandle(RESOURCETYPE_BUFFER);
			if (!mDevice.CreateBuffer(mAllocator.mBuffer, &mAllocator.mDesc, nullptr))
			{
				GPU::DestroyResource(mAllocator.mBuffer);
				Logger::Error("[CommandList] Failed to create buffer.");
				return allocation;
			}
			mDevice.SetResourceName(mAllocator.mBuffer, "CommandListFrameAllocator");
		}

		bool wrap = false;
		size_t position = mAllocator.mOffset;
		if (position + size > mAllocator.mDesc.mByteWidth ||
			mAllocator.mResidentFrame != mDevice.GetFrameCount()) {
			wrap = true;
		}
		position = wrap ? 0 : position;
		
		D3D11_MAP mapping = wrap ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		const HRESULT hr = mDeviceContext->Map(mDevice.mBuffers.Read(mAllocator.mBuffer)->mResource.Get(), 0, mapping, 0, &mappedResource);
		Debug::ThrowIfFailed(SUCCEEDED(hr), "Failed to map buffer of allocator.");

		mAllocator.mResidentFrame = (I32)mDevice.GetFrameCount();
		mAllocator.mOffset = position + size;
		mAllocator.mIsDirty = true;

		return allocation;
	}

	void CommandListDX11::CommitAllactor()
	{
		if (mAllocator.mIsDirty)
		{
			mAllocator.mIsDirty = false;
			mDeviceContext->Unmap(mDevice.mBuffers.Read(mAllocator.mBuffer)->mResource.Get(), 0);
		}
	}

	void CommandListDX11::ActiveFrameBindingSet(const FrameBindingSetDX11* bindingSet)
	{
		mActiveFrameBindingSet = bindingSet;
	}
}
}
