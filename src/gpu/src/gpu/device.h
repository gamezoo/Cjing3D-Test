#pragma once

#include "definitions.h"
#include "commandList.h"

namespace Cjing3D {
namespace GPU
{
	static const U32 MAX_COMMANDLIST_COUNT = 32;

	class GraphicsDevice
	{
	public:
		GraphicsDevice(GraphicsDeviceType type) : mDeviceType(type) {}
		virtual ~GraphicsDevice() {}

		virtual bool CreateCommandlist(ResHandle handle, GPU::CommandListType type) = 0;
		virtual bool CompileCommandList(ResHandle handle, CommandList& cmd) = 0;
		virtual bool SubmitCommandLists(Span<ResHandle> handles) = 0;
		virtual void ResetCommandList(ResHandle handle) = 0;
		virtual void Present(ResHandle handle, bool isVsync) = 0;
		virtual void EndFrame() = 0;

		virtual bool CreateSwapChain(ResHandle handle, const SwapChainDesc* desc, Platform::WindowType window) = 0;
		virtual bool CreateFrameBindingSet(ResHandle handle, const FrameBindingSetDesc* desc) = 0;
		virtual bool CreateTexture(ResHandle handle, const TextureDesc* desc, const SubresourceData* initialData) = 0;
		virtual bool CreateBuffer(ResHandle handle, const BufferDesc* desc, const SubresourceData* initialData) = 0;
		virtual bool CreateShader(ResHandle handle, SHADERSTAGES stage, const void* bytecode, size_t length) = 0;
		virtual bool CreateSamplerState(ResHandle handle, const SamplerDesc* desc) = 0;
		virtual bool CreatePipelineState(ResHandle handle, const PipelineStateDesc* desc) = 0;
		virtual bool CreatePipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc) = 0;
		virtual bool CreateTempPipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc) = 0;
		virtual bool CreateTransientTexture(ResHandle handle, const TextureDesc* desc) = 0;

		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs) = 0;
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs) = 0;
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs) = 0;
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams) = 0;
		virtual bool CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src) = 0;
		
		virtual void DestroyResource(ResHandle handle) = 0;
		virtual void SetResourceName(ResHandle resource, const char* name) = 0;
		virtual GPUAllocation GPUAllcate(ResHandle handle, size_t size) = 0;
		virtual void Map(GPU::ResHandle res, GPUMapping& mapping) = 0;
		virtual void Unmap(GPU::ResHandle res) = 0;

		// add static sampler, it will be valid for the entire rendering pass
		virtual void AddStaticSampler(const StaticSampler& sampler) = 0;

		GraphicsDeviceType GetGraphicsDeviceType()const { return mDeviceType; }
		U64 GetFrameCount()const { return mCurrentFrameCount; }
		U32 GetBackBufferCount()const { return BACK_BUFFER_COUNT; }
		bool CheckCapability(GPU_CAPABILITY capability) { return (U32)capability & mCapabilities; }

	protected:
		static constexpr U32 BACK_BUFFER_COUNT = 2;

		GraphicsDeviceType mDeviceType;
		bool mIsDebug = false;
		U64 mCurrentFrameCount = 0;
		U32 mCapabilities = 0;
	};
}
}