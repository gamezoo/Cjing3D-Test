#pragma once

#include "definitions.h"
#include "resource.h"
#include "device.h"
#include "commandList.h"
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
	void PresentBegin(CommandList& cmd);
	void PresentEnd();
	void EndFrame();
	bool IsHandleValid(ResHandle handle);

	CommandList* CreateCommandlist();
	bool CompileCommandList(CommandList& cmd);
	bool SubmitCommandList(const CommandList& cmd);
	bool SubmitCommandList(Span<CommandList*> cmds);

	ResHandle AllocateHandle(ResourceType type);
	ResHandle CreateFrameBindingSet(const FrameBindingSetDesc* desc);
	ResHandle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData);
	ResHandle CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData);
	ResHandle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length);
	ResHandle CreateSampler(const SamplerDesc* desc);
	ResHandle CreatePipelineState(const PipelineStateDesc* desc);
	ResHandle CreatePipelineBindingSet(const PipelineBindingSetDesc* desc);
	void      DestroyResource(ResHandle handle);

	bool UpdatePipelineBindings(ResHandle handle, I32 slot, Span<BindingSRV> srvs);
	bool UpdatePipelineBindings(ResHandle handle, I32 slot, Span<BindingUAV> uavs);
	bool UpdatePipelineBindings(ResHandle handle, I32 slot, Span<BindingBuffer> cbvs);

	U32  GetFormatStride(FORMAT value);
	bool IsFormatSupportStencil(FORMAT value);
}
}