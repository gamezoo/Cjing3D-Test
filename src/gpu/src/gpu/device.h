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
		GraphicsDevice(GraphicsDeviceType type);
		virtual ~GraphicsDevice();

		virtual bool CreateCommandlist(ResHandle handle) = 0;
		virtual bool CompileCommandList(ResHandle handle, CommandList& cmd) = 0;
		virtual bool SubmitCommandLists(Span<ResHandle> handles) = 0;
		virtual void ResetCommandList(ResHandle handle) = 0;
		virtual void PresentBegin(ResHandle handle) = 0;
		virtual void PresentEnd() = 0;
		virtual void EndFrame() = 0;

		virtual bool CreateFrameBindingSet(ResHandle handle, const FrameBindingSetDesc* desc) = 0;
		virtual bool CreateTexture(ResHandle handle, const TextureDesc* desc, const SubresourceData* initialData) = 0;
		virtual bool CreateBuffer(ResHandle handle, const BufferDesc* desc, const SubresourceData* initialData) = 0;
		virtual bool CreateShader(ResHandle handle, SHADERSTAGES stage, const void* bytecode, size_t length) = 0;
		virtual bool CreateSamplerState(ResHandle handle, const SamplerDesc* desc) = 0;
		virtual bool CreatePipelineState(ResHandle handle, const PipelineStateDesc* desc) = 0;
		virtual bool CreatePipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc) = 0;
		virtual bool CreateTempPipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc) = 0;
		
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs) = 0;
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs) = 0;
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs) = 0;
		virtual bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams) = 0;
		virtual bool CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src) = 0;
		
		virtual void DestroyResource(ResHandle handle) = 0;
		virtual void SetResourceName(ResHandle resource, const char* name) = 0;
		virtual void AddStaticSampler(const StaticSampler& sampler) = 0;

		U32x2 GetResolution()const { return mResolution; }
		U32 GetResolutionWidth()const { return mResolution.x(); }
		U32 GetResolutionHeight()const { return mResolution.y(); }
		F32x2 GetScreenSize()const;

		bool IsFullScreen()const { return mIsFullScreen; }
		bool GetIsVsync()const { return mIsVsync; }
		GraphicsDeviceType GetGraphicsDeviceType()const { return mDeviceType; }
		U64 GetFrameCount()const { return mCurrentFrameCount; }
		FORMAT GetBackBufferFormat()const { return mBackBufferFormat; }
		U32 GetBackBufferCount()const { return mBackBufferCount; }
		bool CheckCapability(GPU_CAPABILITY capability) { return (U32)capability & mCapabilities; }

		void SetIsVsync(bool isVsync) { mIsVsync = isVsync; }
		virtual void SetResolution(const U32x2 size) = 0;

	protected:
		GraphicsDeviceType mDeviceType;
		bool mIsFullScreen = false;
		FORMAT mBackBufferFormat = FORMAT_R8G8B8A8_UNORM;
		U32 mBackBufferCount = 2;
		U32x2 mResolution = U32x2(0u, 0u);
		bool mIsVsync = true;
		bool mIsDebug = false;
		U64 mCurrentFrameCount = 0;
		U32 mCapabilities = 0;
	};
}
}