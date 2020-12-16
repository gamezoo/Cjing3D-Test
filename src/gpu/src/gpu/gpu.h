#pragma once

#include "definitions.h"
#include "resource.h"
#include "device.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
namespace GPU
{
	struct GPUSetupParams
	{
		Platform::WindowType mWindow;
		bool mIsFullscreen;
	};

	void Initialize(GPUSetupParams params);
	bool IsInitialized();
	void Uninitialize();
	GPU::GraphicsDevice* GetDevice();
	void PresentBegin(ResHandle handle);
	void PresentEnd(ResHandle handle);
	void EndFrame();
	bool IsHandleValid(ResHandle handle);

	ResHandle CreateCommandlist();
	bool CompileCommandList(ResHandle handle, const CommandList& cmd);
	bool SubmitCommandList(ResHandle handle);

	ResHandle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData);
	ResHandle CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData);
	ResHandle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length);
	ResHandle CreateInputLayout(const InputLayoutDesc* desc, U32 numElements, ResHandle shader);
	ResHandle CreateSamplerState(const SamplerDesc* desc);
	ResHandle CreatePipelineState(const PipelineStateDesc* desc);
	void   DestroyResource(ResHandle handle);
}
}