#pragma once

#ifdef CJING3D_RENDERER_DX11
#ifdef CJING3D_PLATFORM_WIN32

#include "gpu\device.h"
#include "gpu\dx11\includeDX11.h"
#include "gpu\dx11\resourceDX11.h"
#include "gpu\dx11\allocatorDX11.h"
#include "core\platform\platform.h"
#include "core\container\hashMap.h"

namespace Cjing3D {
namespace GPU
{
	class CompileContextDX11;

	class GraphicsDeviceDx11 : public GraphicsDevice
	{
	public:
		friend class CompileContextDX11;
		friend class CommandListDX11;

		GraphicsDeviceDx11(bool isDebug = false);
		virtual ~GraphicsDeviceDx11();

		bool CreateCommandlist(ResHandle handle, GPU::CommandListType type)override;
		bool CompileCommandList(ResHandle handle, CommandList& cmd)override;
		bool SubmitCommandLists(Span<ResHandle> handles)override;
		void ResetCommandList(ResHandle handle)override;
		void Present(ResHandle handle, bool isVsync)override;
		void EndFrame() override;

		bool CreateSwapChain(ResHandle handle, const SwapChainDesc* desc, Platform::WindowType window)override;
		bool CreateFrameBindingSet(ResHandle handle, const FrameBindingSetDesc* desc)override;
		bool CreateTexture(ResHandle handle, const TextureDesc* desc, const SubresourceData* initialData)override;
		bool CreateTransientTexture(ResHandle handle, const TextureDesc* desc)override;
		bool CreateBuffer(ResHandle handle, const BufferDesc* desc, const SubresourceData* initialData)override;
		bool CreateShader(ResHandle handle, SHADERSTAGES stage, const void* bytecode, size_t length)override;
		bool CreateSamplerState(ResHandle handle, const SamplerDesc* desc)override;
		bool CreatePipelineState(ResHandle handle, const PipelineStateDesc* desc)override;
		bool CreatePipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc)override;
		bool CreateTempPipelineBindingSet(ResHandle handle, const PipelineBindingSetDesc* desc)override;

		bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs)override;
		bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs)override;
		bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs)override;
		bool UpdatePipelineBindingSet(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams)override;
		bool CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src)override;

		void DestroyResource(ResHandle handle)override;
		void SetResourceName(ResHandle resource, const char* name)override;

		// add static sampler, it will be valid for the entire rendering passs
		void AddStaticSampler(const StaticSampler& sampler)override;
		GPUAllocation GPUAllcate(ResHandle handle, size_t size)override;
		void Map(GPU::ResHandle res, GPUMapping& mapping)override;
		void Unmap(GPU::ResHandle res)override;

		ID3D11Device& GetDevice() { return *mDevice.Get(); }
		ID3D11DeviceContext& GetDeviceContext() { return *mImmediateContext.Get(); }

	private:
		bool CreateTextureImpl(TextureDX11& texture, const TextureDesc* desc, const SubresourceData* initialData);
		void DestroyTextureImpl(TextureDX11& texture);
		int CreateSubresourceImpl(TextureDX11& texture, SUBRESOURCE_TYPE type, U32 firstSlice, U32 sliceCount, U32 firstMip, U32 mipCount);
		int CreateSubresourceImpl(BufferDX11& buffer, SUBRESOURCE_TYPE type, U32 offset, U32 size = ~0);

	private:
		friend class ResourceAllocatorDX11;

		ComPtr<ID3D11Device> mDevice;
		ComPtr<ID3D11DeviceContext> mImmediateContext;
		ComPtr<IDXGIFactory2> mFactory;

		// resources
		ResourcePool<SwapChainDX11> mSwapChains;
		ResourcePool<TextureDX11> mTextures;
		ResourcePool<BufferDX11> mBuffers;
		ResourcePool<ShaderDX11> mShaders;
		ResourcePool<SamplerStateDX11> mSamplers;
		ResourcePool<PipelineStateDX11> mPipelineStates;
		ResourcePool<PipelineBindingSetDX11> mPipelineBindingSets;
		ResourcePool<CommandListDX11*> mCommandLists;
		ResourcePool<FrameBindingSetDX11> mFrameBindingSets;

		DynamicArray<StaticSampler> mStaticSamplers;
		UniquePtr<ResourceAllocatorDX11> mTransientResAllocator = nullptr;
	};
}
}
#endif
#endif