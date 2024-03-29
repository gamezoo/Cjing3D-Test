#include "gpu.h"
#include "resource.h"
#include "core\concurrency\concurrency.h"
#include "core\container\staticArray.h"
#include "core\helper\enumTraits.h"
#include "core\event\eventSystem.h"

#ifdef CJING3D_RENDERER_DX11
#include "gpu\dx11\deviceDX11.h"
#elif  CJING3D_RENDERER_DX12
#include "gpu\dx12\deviceDX12.h"
#endif

namespace Cjing3D {
namespace GPU
{
	static const I32 MaxGPUFrames = 4;

	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	class ManagerImpl
	{
	public:
		SharedPtr<GPU::GraphicsDevice> mDevice = nullptr;
		HandleAllocator mHandleAllocator = HandleAllocator((I32)RESOURCETYPE_COUNT);
		StaticArray<DynamicArray<ResHandle>, 4> mReleasedHandles;
		Concurrency::Mutex mMutex;
		U64 mCurrentFrameCount = 0;
		DynamicArray<ResHandle> mTransientHandle;

		// command lists
		StaticArray<CommandList, MAX_COMMANDLIST_COUNT> mAllCmdList;
		volatile I32 mUsedCmdCount = 0;

		// swapchian
		SwapChainDesc mSwapChainDesc;
		GPU::ResHandle mSwapChain;
		Platform::WindowType mWindow;

	public:
		ResHandle AllocHandle(ResourceType type)
		{
			Concurrency::ScopedMutex lock(mMutex);
			return ResHandle(mHandleAllocator.Alloc((I32)type));
		}
		
		ResHandle AllocTransientHandle(ResourceType type)
		{
			Concurrency::ScopedMutex lock(mMutex);
			auto handle = ResHandle(mHandleAllocator.Alloc((I32)type));
			mTransientHandle.push(handle);
			return handle;
		}

		void ClearTransientHandles()
		{
			Concurrency::ScopedMutex lock(mMutex);
			for (ResHandle handle : mTransientHandle)
			{
				mDevice->DestroyResource(handle);
				mHandleAllocator.Free(handle);
			}
			mTransientHandle.clear();
		}

		void ProcessReleasedHandles()
		{
			Concurrency::ScopedMutex lock(mMutex);
			auto& releasedHandles = mReleasedHandles[mCurrentFrameCount % MaxGPUFrames];
			for (ResHandle handle : releasedHandles)
			{
				mDevice->DestroyResource(handle);
				mHandleAllocator.Free(handle);
			}
			releasedHandles.clear();
		}

		bool CheckHandle(ResHandle& handle, bool ret)
		{
			if (!ret)
			{
				mHandleAllocator.Free(handle);
				handle = ResHandle::INVALID_HANDLE;
			}
			return ret;
		}

		void DestroyHandle(ResHandle handle)
		{
			Concurrency::ScopedMutex lock(mMutex);
			auto& releasedHandles = mReleasedHandles[mCurrentFrameCount % MaxGPUFrames];
			releasedHandles.push(handle);
		}
	};
	ManagerImpl* mImpl = nullptr;

	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////

	// DEBUG INFOS
#ifdef DEBUG
	struct ResourceDebugInfo
	{
		ResourceDebugInfo() = default;
		ResourceDebugInfo(ResHandle handle, const char* name) :
			mHandle(handle)
		{
			mName.Sprintf("%s [%u %u]", name, handle.GetIndex(), handle.GetType());
		}

		StaticString<64> mName;
		ResHandle mHandle;
	};
	
	Concurrency::Mutex debugInfoMutex;
	StaticArray<DynamicArray<ResourceDebugInfo>, GPU::RESOURCETYPE_COUNT> mResourceDebugInfos;
	void SetDebugInfo(ResHandle handle, const char* name)
	{
		Concurrency::ScopedMutex lock(debugInfoMutex);
		auto& debugInfos = mResourceDebugInfos[(I32)handle.GetType()];
		if (debugInfos.size() <= handle.GetIndex()) {
			debugInfos.resize(PotRoundUp((I32)handle.GetIndex() + 1, 32));
		}
		debugInfos[handle.GetIndex()] = ResourceDebugInfo(handle, name);
	}

	ResourceDebugInfo GetDebugInfo(ResHandle handle)
	{
		return mResourceDebugInfos[(I32)handle.GetType()][handle.GetIndex()];
	}

#define SET_DEBUG_NAME(name)														\
	if (name != nullptr && handle != ResHandle::INVALID_HANDLE) {					\
		mImpl->mDevice->SetResourceName(handle, name);								\
		SetDebugInfo(handle, name);													\
	}
#else
#define SET_DEBUG_NAME(name) static const char* debugName = "";
#endif

	void Initialize(GPUSetupParams params)
	{
		if (IsInitialized()) {
			return;
		}

		// initialize graphics device
#ifdef DEBUG
		bool debug = true;
#else
		bool debug = false;
#endif
#ifdef CJING3D_RENDERER_DX11
		SharedPtr<GPU::GraphicsDevice> device = CJING_MAKE_SHARED<GPU::GraphicsDeviceDx11>(debug);
#else
		Logger::Error("Unsupport graphics device");
#endif
		mImpl = CJING_NEW(ManagerImpl);
		mImpl->mDevice = device;
		mImpl->mWindow = params.mWindow;

		// create swapChain
		auto clientBounds = Platform::GetClientBounds(params.mWindow);
		SwapChainDesc desc = {};
		desc.mWidth  = clientBounds.mRight - clientBounds.mLeft;
		desc.mHeight = clientBounds.mBottom - clientBounds.mTop;
		desc.mBufferCount = 2;
		desc.mFormat = FORMAT_R8G8B8A8_UNORM;

		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_SWAP_CHAIN);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateSwapChain(handle, &desc, params.mWindow));
		if (!handle) {
			Logger::Error("Failed to create swap chain");
		}
		mImpl->mSwapChain = handle;
		mImpl->mSwapChainDesc = desc;
	}

	bool IsInitialized()
	{
		return mImpl != nullptr;
	}

	void Uninitialize()
	{
		if (!IsInitialized()) {
			return;
		}

		// release command list handle
		for (int i = 0; i < MAX_COMMANDLIST_COUNT; i++)
		{
			auto handle = mImpl->mAllCmdList[i].GetHanlde();
			if (handle != ResHandle::INVALID_HANDLE)
			{
				mImpl->mDevice->DestroyResource(handle);
				mImpl->mHandleAllocator.Free(handle);
			}
		}
		mImpl->mUsedCmdCount = 0;
		mImpl->ClearTransientHandles();

		// clear swapChain
		mImpl->DestroyHandle(mImpl->mSwapChain);

		// process release handles
		for (int i = 0; i < MaxGPUFrames; i++) 
		{
			mImpl->mCurrentFrameCount++;
			mImpl->ProcessReleasedHandles();
		}

#ifdef DEBUG
		I32 aliveHandles = false;
		for (I32 i = 0; i < (I32)ResourceType::RESOURCETYPE_COUNT; i++) {
			aliveHandles += mImpl->mHandleAllocator.GetTotalHandleCount(i);
		}

		if (aliveHandles > 0)
		{
			std::stringstream os;
			os << std::endl;
			os << "[GPU] Detected handles leaks !!! " << std::endl;
			os << "[GPU] Leaked handles usage:" << aliveHandles << std::endl;
			os << "[GPU] Dumping handles:" << std::endl;

			auto& handles = mImpl->mHandleAllocator;
			for (I32 i = 0; i < (I32)ResourceType::RESOURCETYPE_COUNT; i++) 
			{
				if(handles.GetTotalHandleCount(i) > 0)
				{
					os << "- Type:" << EnumTraits::EnumToName(ResourceType(i)) << std::endl;
					for (I32 resIndex = 0; resIndex < handles.GetMaxHandleCount(i); resIndex++)
					{
						if (handles.IsAllocated(i, resIndex))
						{
							os << "- - ResHandle:" << mResourceDebugInfos[i][resIndex].mName << std::endl;
						}
					}
				}
			}

			Logger::Warning(os.str().c_str());

			system("pause");
		}

		for (I32 i = 0; i < (I32)ResourceType::RESOURCETYPE_COUNT; i++) {
			mResourceDebugInfos[i].free();
		}
#endif
		CJING_SAFE_DELETE(mImpl);
	}

	GPU::GraphicsDevice* GetDevice()
	{
		return mImpl->mDevice.get();
	}

	void EndFrame()
	{
		// do ending frame jobs
		mImpl->mCurrentFrameCount++;
		mImpl->mDevice->EndFrame();
		mImpl->ProcessReleasedHandles();
		mImpl->ClearTransientHandles();
	}

	bool IsHandleValid(ResHandle handle)
	{
		return mImpl->mHandleAllocator.IsValid(handle);
	}

	void Present()
	{
		// first submit all remain command lists
		SubmitCommandLists();

		// swapchain present
		mImpl->mDevice->Present(mImpl->mSwapChain, mImpl->mSwapChainDesc.mVsync);
	}

	void ResizeSwapChain(ResHandle handle, U32 width, U32 height)
	{
		auto& desc = mImpl->mSwapChainDesc;
		desc.mWidth = width;
		desc.mHeight = height;
		mImpl->mDevice->CreateSwapChain(handle, &desc, mImpl->mWindow);
	}

	CommandList* CreateCommandlist(GPU::CommandListType type)
	{
		I32 cmdIndex = Concurrency::AtomicIncrement(&mImpl->mUsedCmdCount) - 1;
		CommandList* cmd = &mImpl->mAllCmdList[cmdIndex];
		if (cmd->GetHanlde() == ResHandle::INVALID_HANDLE)
		{
			ResHandle handle = mImpl->AllocHandle(RESOURCETYPE_COMMAND_LIST);
			bool ret = mImpl->CheckHandle(handle, mImpl->mDevice->CreateCommandlist(handle, type));
			if (!ret) {
				return nullptr;
			}
			cmd->SetHanlde(handle);
		}
		else 
		{
			cmd->Reset();
		}

		mImpl->mDevice->ResetCommandList(cmd->GetHanlde());

		return cmd;
	}

	bool CompileCommandList(CommandList& cmd)
	{
		Debug::CheckAssertion(cmd.GetHanlde() != ResHandle::INVALID_HANDLE);
		if (!cmd.IsCompiled())
		{
			cmd.SetCompiled(true);
			return mImpl->mDevice->CompileCommandList(cmd.GetHanlde(), cmd);
		}
		return false;
	}

	bool SubmitCommandLists()
	{
		U32 count = mImpl->mUsedCmdCount;
		Concurrency::AtomicExchange(&mImpl->mUsedCmdCount, 0);
		mImpl->mUsedCmdCount = 0;
		if (count > 0)
		{
			DynamicArray<ResHandle> handles;
			for (U32 i = 0; i < count; i++)
			{
				auto& cmd = mImpl->mAllCmdList[i];
				auto handle = cmd.GetHanlde();
				if (handle != ResHandle::INVALID_HANDLE)
				{
					if (!cmd.IsCompiled()) {
						mImpl->mDevice->CompileCommandList(handle, cmd);
					}
					handles.push(handle);
				}
			}
			mImpl->mDevice->SubmitCommandLists(Span<ResHandle>(handles.data(), handles.size()));
		}
		return true;
	}

	ResHandle AllocateHandle(ResourceType type)
	{
		return mImpl->AllocHandle(type);
	}

	ResHandle CreateFrameBindingSet(const FrameBindingSetDesc* desc, const char* name)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_FRAME_BINDING_SET);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateFrameBindingSet(handle, desc));
		SET_DEBUG_NAME(name);
		return handle;
	}

	ResHandle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData, const char* name)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_TEXTURE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateTexture(handle, desc, initialData));
		SET_DEBUG_NAME(name);
		return handle;
	}

	ResHandle CreateBuffer(const BufferDesc* desc, const SubresourceData* initialData, const char* name)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_BUFFER);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateBuffer(handle, desc, initialData));
		SET_DEBUG_NAME(name);
		return handle;
	}

	ResHandle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_SHADER);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateShader(handle, stage, bytecode, length));
		return handle;
	}

	ResHandle CreateSampler(const SamplerDesc* desc, const char* name)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_SAMPLER_STATE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateSamplerState(handle, desc));
		SET_DEBUG_NAME(name);
		return handle;
	}

	ResHandle CreatePipelineState(const PipelineStateDesc* desc)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_PIPELINE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreatePipelineState(handle, desc));
		return handle;
	}

	ResHandle CreatePipelineBindingSet(const PipelineBindingSetDesc* desc)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_PIPELINE_BINDING_SET);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreatePipelineBindingSet(handle, desc));
		return handle;
	}

	ResHandle CreateTempPipelineBindingSet(const PipelineBindingSetDesc* desc)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_PIPELINE_BINDING_SET);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreatePipelineBindingSet(handle, desc));
		// temporary resource, is will be destroyed in the next frame
		DestroyResource(handle);
		return handle;
	}


	ResHandle CreateTransientTexture(const TextureDesc* desc)
	{
		ResHandle handle = mImpl->AllocTransientHandle(ResourceType::RESOURCETYPE_TEXTURE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateTransientTexture(handle, desc));
		return handle;
	}

	void DestroyResource(ResHandle handle)
	{
		if (handle && IsHandleValid(handle)) {
			mImpl->DestroyHandle(handle);
		}
	}

	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingSRV> srvs)
	{
		Debug::CheckAssertion(handle.GetType() == RESOURCETYPE_PIPELINE_BINDING_SET);
		return mImpl->CheckHandle(handle, mImpl->mDevice->UpdatePipelineBindingSet(handle, index, slot, srvs));
	}

	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingUAV> uavs)
	{
		Debug::CheckAssertion(handle.GetType() == RESOURCETYPE_PIPELINE_BINDING_SET);
		return mImpl->CheckHandle(handle, mImpl->mDevice->UpdatePipelineBindingSet(handle, index, slot, uavs));
	}

	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingBuffer> cbvs)
	{
		Debug::CheckAssertion(handle.GetType() == RESOURCETYPE_PIPELINE_BINDING_SET);
		return mImpl->CheckHandle(handle, mImpl->mDevice->UpdatePipelineBindingSet(handle, index, slot, cbvs));
	}

	bool UpdatePipelineBindings(ResHandle handle, I32 index, I32 slot, Span<const BindingSAM> sams)
	{
		Debug::CheckAssertion(handle.GetType() == RESOURCETYPE_PIPELINE_BINDING_SET);
		return mImpl->CheckHandle(handle, mImpl->mDevice->UpdatePipelineBindingSet(handle, index, slot, sams));
	}

	void CopyPipelineBindings(const PipelineBinding& dst, const PipelineBinding& src)
	{
		if (!mImpl->mDevice->CopyPipelineBindings(dst, src)) {
			Logger::Warning("Failed to copy pipelineBindingSet");
		}
	}

	void AddStaticSampler(const StaticSampler& sampler)
	{
		mImpl->mDevice->AddStaticSampler(sampler);
	}

	GPUAllocation GPUAllcate(CommandList& cmd, size_t size)
	{
		Debug::CheckAssertion(cmd.GetHanlde() != ResHandle::INVALID_HANDLE);
		return mImpl->mDevice->GPUAllcate(cmd.GetHanlde(), size);
	}

	void Map(GPU::ResHandle res, GPUMapping& mapping)
	{
		mImpl->mDevice->Map(res, mapping);
	}

	void Unmap(GPU::ResHandle res)
	{
		mImpl->mDevice->Unmap(res);
	}

	ResHandle GetSwapChain()
	{
		return mImpl->mSwapChain;
	}

	SwapChainDesc& GetSwapChainDesc()
	{
		return mImpl->mSwapChainDesc;
	}

	FORMAT GetBackBufferFormat()
	{
		return mImpl->mSwapChainDesc.mFormat;
	}

	U32 GetFormatStride(FORMAT value)
	{
		switch (value)
		{
		case FORMAT_R32G32B32A32_FLOAT:
		case FORMAT_R32G32B32A32_UINT:
		case FORMAT_R32G32B32A32_SINT:
			return 16;

		case FORMAT_R32G32B32_FLOAT:
		case FORMAT_R32G32B32_UINT:
		case FORMAT_R32G32B32_SINT:
			return 12;

		case FORMAT_R16G16B16A16_FLOAT:
		case FORMAT_R16G16B16A16_UNORM:
		case FORMAT_R16G16B16A16_UINT:
		case FORMAT_R16G16B16A16_SNORM:
		case FORMAT_R16G16B16A16_SINT:
			return 8;

		case FORMAT_R32G32_FLOAT:
		case FORMAT_R32G32_UINT:
		case FORMAT_R32G32_SINT:
		case FORMAT_R32G8X24_TYPELESS:
		case FORMAT_D32_FLOAT_S8X24_UINT:
			return 8;

		case FORMAT_R10G10B10A2_UNORM:
		case FORMAT_R10G10B10A2_UINT:
		case FORMAT_R11G11B10_FLOAT:
		case FORMAT_R8G8B8A8_UNORM:
		case FORMAT_R8G8B8A8_UNORM_SRGB:
		case FORMAT_R8G8B8A8_UINT:
		case FORMAT_R8G8B8A8_SNORM:
		case FORMAT_R8G8B8A8_SINT:
		case FORMAT_B8G8R8A8_UNORM:
		case FORMAT_B8G8R8A8_UNORM_SRGB:
		case FORMAT_R16G16_FLOAT:
		case FORMAT_R16G16_UNORM:
		case FORMAT_R16G16_UINT:
		case FORMAT_R16G16_SNORM:
		case FORMAT_R16G16_SINT:
		case FORMAT_R32_TYPELESS:
		case FORMAT_D32_FLOAT:
		case FORMAT_R32_FLOAT:
		case FORMAT_R32_UINT:
		case FORMAT_R32_SINT:
		case FORMAT_R24G8_TYPELESS:
		case FORMAT_D24_UNORM_S8_UINT:
			return 4;

		case FORMAT_R8G8_UNORM:
		case FORMAT_R8G8_UINT:
		case FORMAT_R8G8_SNORM:
		case FORMAT_R8G8_SINT:
		case FORMAT_R16_TYPELESS:
		case FORMAT_R16_FLOAT:
		case FORMAT_D16_UNORM:
		case FORMAT_R16_UNORM:
		case FORMAT_R16_UINT:
		case FORMAT_R16_SNORM:
		case FORMAT_R16_SINT:
			return 2;

		case FORMAT_R8_UNORM:
		case FORMAT_R8_UINT:
		case FORMAT_R8_SNORM:
		case FORMAT_R8_SINT:
			return 1;

		default:
			break;
		}
		return 16;
	}

	bool IsFormatSupportStencil(FORMAT value)
	{
		switch (value)
		{
		case FORMAT_R32G8X24_TYPELESS:
		case FORMAT_D32_FLOAT_S8X24_UINT:
		case FORMAT_R24G8_TYPELESS:
		case FORMAT_D24_UNORM_S8_UINT:
			return true;
		}
		return false;
	}

	FormatInfo GetFormatInfo(FORMAT format)
	{
		FormatInfo info;
		switch (format)
		{
		case FORMAT_R32G32B32A32_TYPELESS:
		case FORMAT_R32G32B32A32_FLOAT:
		case FORMAT_R32G32B32A32_UINT:
		case FORMAT_R32G32B32A32_SINT:
			info.mRBits = 32;
			info.mGBits = 32;
			info.mBBits = 32;
			info.mABits = 32;
			info.mChannels = 4;
			break;
		case FORMAT_R32G32B32_TYPELESS:
		case FORMAT_R32G32B32_FLOAT:
		case FORMAT_R32G32B32_UINT:
		case FORMAT_R32G32B32_SINT:
			info.mRBits = 32;
			info.mGBits = 32;
			info.mBBits = 32;
			info.mChannels = 3;
			break;
		case FORMAT_R16G16B16A16_TYPELESS:
		case FORMAT_R16G16B16A16_FLOAT:
		case FORMAT_R16G16B16A16_UNORM:
		case FORMAT_R16G16B16A16_UINT:
		case FORMAT_R16G16B16A16_SNORM:
		case FORMAT_R16G16B16A16_SINT:
			info.mRBits = 16;
			info.mGBits = 16;
			info.mBBits = 16;
			info.mABits = 16;
			info.mChannels = 4;
			break;
		case FORMAT_R32G32_TYPELESS:
		case FORMAT_R32G32_FLOAT:
		case FORMAT_R32G32_UINT:
		case FORMAT_R32G32_SINT:
			info.mRBits = 32;
			info.mGBits = 32;
			info.mChannels = 2;
			break;
		case FORMAT_R10G10B10A2_UNORM:
		case FORMAT_R10G10B10A2_UINT:
			info.mRBits = 10;
			info.mGBits = 10;
			info.mBBits = 10;
			info.mABits = 2;
			info.mChannels = 4;
			break;
		case FORMAT_R11G11B10_FLOAT:
			info.mRBits = 10;
			info.mGBits = 10;
			info.mBBits = 10;
			info.mABits = 2;
			info.mChannels = 4;
			break;
		case FORMAT_R8G8B8A8_UNORM:
		case FORMAT_R8G8B8A8_UNORM_SRGB:
		case FORMAT_R8G8B8A8_UINT:
		case FORMAT_R8G8B8A8_SNORM:
		case FORMAT_R8G8B8A8_SINT:
		case FORMAT_B8G8R8A8_UNORM:
		case FORMAT_B8G8R8A8_UNORM_SRGB:
			info.mRBits = 8;
			info.mGBits = 8;
			info.mBBits = 8;
			info.mABits = 8;
			info.mChannels = 4;
			break;
		case FORMAT_R16G16_FLOAT:
		case FORMAT_R16G16_UNORM:
		case FORMAT_R16G16_UINT:
		case FORMAT_R16G16_SNORM:
		case FORMAT_R16G16_SINT:
			info.mRBits = 16;
			info.mGBits = 16;
			info.mChannels = 2;
			break;
		case FORMAT_R32_TYPELESS:
		case FORMAT_R32_FLOAT:
		case FORMAT_R32_UINT:
		case FORMAT_R32_SINT:
			info.mRBits = 32;
			info.mChannels = 1;
			break;
		case FORMAT_D32_FLOAT:
			info.mDBits = 32;
			info.mChannels = 1;
			break;
		case FORMAT_D24_UNORM_S8_UINT:
			info.mDBits = 24;
			info.mSBits = 8;
			info.mChannels = 2;
			break;
		case FORMAT_R24G8_TYPELESS:
			info.mRBits = 24;
			info.mGBits = 8;
			info.mChannels = 2;
			break;
		case FORMAT_R8G8_UNORM:
		case FORMAT_R8G8_UINT:
		case FORMAT_R8G8_SNORM:
		case FORMAT_R8G8_SINT:
			info.mRBits = 8;
			info.mGBits = 8;
			info.mChannels = 2;
			break;
		case FORMAT_R16_TYPELESS:
		case FORMAT_R16_FLOAT:
		case FORMAT_D16_UNORM:
		case FORMAT_R16_UNORM:
		case FORMAT_R16_UINT:
		case FORMAT_R16_SNORM:
		case FORMAT_R16_SINT:
			info.mRBits = 16;
			info.mChannels = 1;
			break;
		case FORMAT_R8_UNORM:
		case FORMAT_R8_UINT:
		case FORMAT_R8_SNORM:
		case FORMAT_R8_SINT:
			info.mRBits = 8;
			info.mChannels = 1;
			break;
		default:
			break;
		}

		info.mBlockBits += info.mRBits;
		info.mBlockBits += info.mGBits;
		info.mBlockBits += info.mBBits;
		info.mBlockBits += info.mABits;
		info.mBlockBits += info.mDBits;
		info.mBlockBits += info.mSBits;
		info.mBlockBits += info.mXBits;
		info.mBlockBits += info.mEBits;

		//if no block bits, must be a compressed format.
		if (info.mBlockBits == 0)
		{
			switch (format)
			{
			case FORMAT_BC1_TYPELESS:
			case FORMAT_BC1_UNORM:
			case FORMAT_BC1_UNORM_SRGB:
			case FORMAT_BC4_TYPELESS:
			case FORMAT_BC4_UNORM:
			case FORMAT_BC4_SNORM:
				info.mBlockBits = 64;
				info.mBlockW = 4;
				info.mBlockH = 4;
				break;
			case FORMAT_BC2_TYPELESS:
			case FORMAT_BC2_UNORM:
			case FORMAT_BC2_UNORM_SRGB:
			case FORMAT_BC3_TYPELESS:
			case FORMAT_BC3_UNORM:
			case FORMAT_BC3_UNORM_SRGB:
			case FORMAT_BC5_TYPELESS:
			case FORMAT_BC5_UNORM:
			case FORMAT_BC5_SNORM:
			case FORMAT_BC6H_TYPELESS:
			case FORMAT_BC6H_UF16:
			case FORMAT_BC6H_SF16:
			case FORMAT_BC7_TYPELESS:
			case FORMAT_BC7_UNORM:
			case FORMAT_BC7_UNORM_SRGB:
				info.mBlockBits = 128;
				info.mBlockW = 4;
				info.mBlockH = 4;
				break;
			default:
				Logger::Error("Unsupport format:%d", format);
				return FormatInfo();
			}
		}
		else
		{
			info.mBlockW = 1;
			info.mBlockH = 1;
		}

		// Just handle this one separately.
		if (format == FORMAT_R1_UNORM)
		{
			info.mBlockW = 8;
			info.mBlockBits = 8;
		}

		return info;
	}

	TextureLayoutInfo GetTextureLayoutInfo(FORMAT format, I32 width, I32 height)
	{
		FormatInfo formatInfo = GetFormatInfo(format);
		I32 widthByBlock  = std::max(1, width / formatInfo.mBlockW);
		I32 heightByBlock = std::max(1, height / formatInfo.mBlockH);

		return {
			widthByBlock * formatInfo.mBlockBits / 8,
			widthByBlock * heightByBlock * formatInfo.mBlockBits / 8
		};
	}

	U32 GetTextureSize(FORMAT format, I32 width, I32 height, I32 depth, I32 mipLevel)
	{
		U32 size = 0;
		for (I32 level = 0; level < mipLevel; level++)
		{
			FormatInfo formatInfo = GetFormatInfo(format);
			I32 blockW = PotRoundUp(width, formatInfo.mBlockW) / formatInfo.mBlockW;
			I32 blockH = PotRoundUp(height, formatInfo.mBlockH) / formatInfo.mBlockH;
			I32 blockD = depth;

			size += (formatInfo.mBlockBits * blockW * blockH * blockD) / 8;
			width  = std::max(width / 2, 1);
			height = std::max(height / 2, 1);
			depth  = std::max(depth / 2, 1);
		}
		return size;
	}
}
}