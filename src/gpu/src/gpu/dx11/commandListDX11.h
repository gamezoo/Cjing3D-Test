#pragma once

#include "includeDX11.h"
#include "resourceDX11.h"
#include "gpu\commandList.h"

namespace Cjing3D
{
namespace GPU
{
	class CommandListDX11
	{
	public:
		CommandListDX11(ID3D11Device& device);
		~CommandListDX11();

		ID3D11DeviceContext* GetContext();
		ID3D11CommandList* GetCommandList();

		bool Submit(ID3D11DeviceContext& immediateContext);

	private:
		ComPtr<ID3D11DeviceContext> mDeviceContext;
		ComPtr<ID3D11CommandList> mCommandList;
		ComPtr<ID3DUserDefinedAnnotation> mUserDefinedAnnotations;
	};
}
}