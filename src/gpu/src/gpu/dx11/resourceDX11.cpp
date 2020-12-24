#include "resourceDX11.h"
#include "deviceDX11.h"
#include "gpu\commandList.h"

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
			Debug::Error("[CommandList] Failed to create deferred context: %08X", hr);
			return;
		}

		hr = mDeviceContext.As(&mUserDefinedAnnotations);
		if (FAILED(hr))
		{
			Debug::Error("[CommandList] Failed to set user defined annotations: %08X", hr);
			return;
		}
	}

	CommandListDX11::~CommandListDX11()
	{
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
		ID3D11PixelShader* ps = desc.mPS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mVS)->mPS.Get() : nullptr;
		if (ps != mPrevPixelShader)
		{
			mDeviceContext->PSSetShader(ps, nullptr, 0);
			mPrevPixelShader = ps;
		}
		ID3D11HullShader* hs = desc.mHS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mVS)->mHS.Get() : nullptr;
		if (hs != mPrevHullShader)
		{
			mDeviceContext->HSSetShader(hs, nullptr, 0);
			mPrevHullShader = hs;
		}
		ID3D11DomainShader* ds = desc.mDS != ResHandle::INVALID_HANDLE ? mDevice.mShaders.Read(desc.mVS)->mDS.Get() : nullptr;
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
}
}