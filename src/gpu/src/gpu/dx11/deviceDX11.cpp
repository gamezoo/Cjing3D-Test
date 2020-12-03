#include "deviceDX11.h"

namespace Cjing3D
{
	GraphicsDeviceDx11::GraphicsDeviceDx11(Platform::WindowType window, bool isFullScreen, bool isDebug) :
		GraphicsDevice(GraphicsDeviceType::GraphicsDeviceType_Dx11)
	{
	}

	GraphicsDeviceDx11::~GraphicsDeviceDx11()
	{
	}
	Handle GraphicsDeviceDx11::CreateCommandlist()
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::SubmitCommandList(Handle handle)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateTexture(const TextureDesc* desc, const SubresourceData* initialData)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateDepthStencilState(const DepthStencilStateDesc* desc)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateBlendState(const BlendStateDesc* desc)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateRasterizerState(const RasterizerStateDesc* desc)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateInputLayout(const InputLayoutDesc* desc, U32 numElements, Handle shader)
	{
		return Handle();
	}
	Handle GraphicsDeviceDx11::CreateSamplerState(const SamplerDesc* desc)
	{
		return Handle();
	}
	void GraphicsDeviceDx11::SetResourceName(Handle resource, const char* name)
	{
	}
	void GraphicsDeviceDx11::SetResolution(const U32x2 size)
	{
	}
}