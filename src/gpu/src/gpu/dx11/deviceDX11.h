#pragma once

#include "gpu\device.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
	class GraphicsDeviceDx11 : public GraphicsDevice
	{
	public:
		GraphicsDeviceDx11(Platform::WindowType window, bool isFullScreen = false, bool isDebug = false);
		~GraphicsDeviceDx11();

		Handle CreateCommandlist()override;
		Handle SubmitCommandList(Handle handle)override;

		Handle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData)override;
		Handle CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData)override;
		Handle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length)override;
		Handle CreateDepthStencilState(const DepthStencilStateDesc* desc)override;
		Handle CreateBlendState(const BlendStateDesc* desc)override;
		Handle CreateRasterizerState(const RasterizerStateDesc* desc)override;
		Handle CreateInputLayout(const InputLayoutDesc* desc, U32 numElements, Handle shader)override;
		Handle CreateSamplerState(const SamplerDesc* desc)override;

		void SetResourceName(Handle resource, const char* name)override;
		void SetResolution(const U32x2 size)override;
	};
}