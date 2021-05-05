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
	void EndFrame();
	bool IsHandleValid(ResHandle handle);
	void Present();
	void ResizeSwapChain(ResHandle handle, U32 width, U32 height);

	CommandList* CreateCommandlist(GPU::CommandListType type = GPU::COMMAND_LIST_GRAPHICS);
	bool CompileCommandList(CommandList& cmd);
	bool SubmitCommandList(const CommandList& cmd);
	bool SubmitCommandList(Span<CommandList*> cmds);
	bool SubmitAllRemainCommandList();

	ResHandle AllocateHandle(ResourceType type);
	ResHandle CreateFrameBindingSet(const FrameBindingSetDesc* desc, const char* name = nullptr);
	ResHandle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData, const char* name = nullptr);
	ResHandle CreateBuffer(const BufferDesc* desc, const SubresourceData* initialData, const char* name = nullptr);
	ResHandle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length);
	ResHandle CreateSampler(const SamplerDesc* desc, const char* name = nullptr);
	ResHandle CreatePipelineState(const PipelineStateDesc* desc);
	ResHandle CreatePipelineBindingSet(const PipelineBindingSetDesc* desc);
	ResHandle CreateTempPipelineBindingSet(const PipelineBindingSetDesc* desc);
	ResHandle CreateTransientTexture(const TextureDesc* desc);
	void      DestroyResource(ResHandle handle);

	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs);
	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs);
	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs);
	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams);
	void CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src);

	void AddStaticSampler(const StaticSampler& sampler);
	GPUAllocation GPUAllcate(CommandList& cmd, size_t size);
	void Map(GPU::ResHandle res, GPUMapping& mapping);
	void Unmap(GPU::ResHandle res);

	ResHandle GetSwapChain();
	SwapChainDesc& GetSwapChainDesc();
	FORMAT GetBackBufferFormat();

	U32  GetFormatStride(FORMAT value);
	bool IsFormatSupportStencil(FORMAT value);
	FormatInfo GetFormatInfo(FORMAT format);
	TextureLayoutInfo GetTextureLayoutInfo(FORMAT format, I32 width, I32 height);
	U32 GetTextureSize(FORMAT format, I32 width, I32 height, I32 depth, I32 mipLevel);
}
}