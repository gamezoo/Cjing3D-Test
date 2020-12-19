#include "commandListDX11.h"

namespace Cjing3D
{
namespace GPU
{
	CommandListDX11::CommandListDX11(ID3D11Device& device)
	{
		HRESULT hr = device.CreateDeferredContext(0, &mDeviceContext);
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

	bool CommandListDX11::Submit(ID3D11DeviceContext& immediateContext)
	{
		if (FAILED(mDeviceContext->FinishCommandList(false, &mCommandList))) {
			return false;
		}
		immediateContext.ExecuteCommandList(mCommandList.Get(), false);
		mCommandList.Reset();
		return true;
	}
}
}
