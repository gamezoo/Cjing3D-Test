#pragma once

#ifdef CJING3D_RENDERER_DX11
#ifdef CJING3D_PLATFORM_WIN32

#include "gpu\device.h"
#include "gpu\dx11\includeDX11.h"
#include "gpu\dx11\resourceDX11.h"
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

		GraphicsDeviceDx11(Platform::WindowType window, bool isFullScreen = false, bool isDebug = false);
		virtual ~GraphicsDeviceDx11();

		bool CreateCommandlist(ResHandle handle)override;
		bool CompileCommandList(ResHandle handle, CommandList& cmd)override;
		bool SubmitCommandLists(Span<ResHandle> handles)override;
		void ResetCommandList(ResHandle handle)override;
		void PresentBegin(ResHandle handle)override;
		void PresentEnd()override;
		void EndFrame() override;

		bool CreateFrameBindingSet(ResHandle handle, const FrameBindingSetDesc* desc)override;
		bool CreateTexture(ResHandle handle, const TextureDesc* desc, const SubresourceData* initialData)override;
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
		void SetResolution(const U32x2 size)override;

		void AddStaticSampler(const StaticSampler& sampler)override;
		GPUAllocation GPUAllcate(ResHandle handle, size_t size)override;

		ID3D11Device& GetDevice() { return *mDevice.Get(); }
		ID3D11DeviceContext& GetDeviceContext() { return *mImmediateContext.Get(); }

	private:
		int CreateSubresourceImpl(TextureDX11& texture, SUBRESOURCE_TYPE type, U32 firstSlice, U32 sliceCount, U32 firstMip, U32 mipCount);
		int CreateSubresourceImpl(BufferDX11& buffer, SUBRESOURCE_TYPE type, U32 offset, U32 size = ~0);

	private:
		ComPtr<ID3D11Device> mDevice;
		ComPtr<IDXGISwapChain1> mSwapChain;
		ComPtr<ID3D11DeviceContext> mImmediateContext;
		ComPtr<ID3D11Texture2D> mBackBuffer;
		ComPtr<ID3D11RenderTargetView> mRenderTargetView;

		// resources
		ResourcePool<TextureDX11> mTextures;
		ResourcePool<BufferDX11> mBuffers;
		ResourcePool<ShaderDX11> mShaders;
		ResourcePool<SamplerStateDX11> mSamplers;
		ResourcePool<PipelineStateDX11> mPipelineStates;
		ResourcePool<PipelineBindingSetDX11> mPipelineBindingSets;
		ResourcePool<CommandListDX11*> mCommandLists;
		ResourcePool<FrameBindingSetDX11> mFrameBindingSets;

		DynamicArray<StaticSampler> mStaticSamplers;
	};
}
}
#endif
#endif