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
	ResHandle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData, const char* name = nullptr);
	ResHandle CreateBuffer(const BufferDesc* desc, const SubresourceData* initialData, const char* name = nullptr);
	ResHandle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length);
	ResHandle CreateSampler(const SamplerDesc* desc, const char* name = nullptr);
	ResHandle CreatePipelineState(const PipelineStateDesc* desc);
	ResHandle CreatePipelineBindingSet(const PipelineBindingSetDesc* desc);
	ResHandle CreateTempPipelineBindingSet(const PipelineBindingSetDesc* desc);
	void      DestroyResource(ResHandle handle);

	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs);
	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs);
	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs);
	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams);
	void CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src);

	void AddStaticSampler(const StaticSampler& sampler);
	GPUAllocation GPUAllcate(CommandList& cmd, size_t size);

	const BufferDesc* GetBufferDesc(ResHandle handle);
	const TextureDesc* GetTextureDesc(ResHandle handle);

	U32  GetFormatStride(FORMAT value);
	bool IsFormatSupportStencil(FORMAT value);
	U32x2 GetResolution();
	F32x2 GetScreenSize();

}
}