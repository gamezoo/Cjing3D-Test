#include "gpu.h"
#include "resource.h"
#include "core\jobsystem\concurrency.h"
#include "core\container\staticArray.h"
#include "core\helper\enumTraits.h"

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

	public:
		ResHandle AllocHandle(ResourceType type)
		{
			Concurrency::ScopedMutex lock(mMutex);
			return ResHandle(mHandleAllocator.Alloc((I32)type));
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
	void Initialize(GPUSetupParams params)
	{
		if (IsInitialized()) {
			return;
		}
		
		Platform::WindowType window = params.mWindow;
		bool fullscreen = params.mIsFullscreen;
#ifdef DEBUG
		bool debug = true;
#else
		bool debug = false;
#endif
#ifdef CJING3D_RENDERER_DX11
		SharedPtr<GPU::GraphicsDevice> device = CJING_MAKE_SHARED<GPU::GraphicsDeviceDx11>(window, fullscreen, debug);
#else
		Debug::Error("Unsupport graphics device");
#endif
		mImpl = CJING_NEW(ManagerImpl);
		mImpl->mDevice = device;
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
							// TODO: get handle debug info
							os << "- - ResHandle index:" << resIndex << std::endl;
						}
					}
				}
			}

			Debug::Warning(os.str().c_str());
#ifdef DEBUG
			system("pause");
#endif // DEBUG

		}
#endif

		CJING_SAFE_DELETE(mImpl);
	}

	GPU::GraphicsDevice* GetDevice()
	{
		return mImpl->mDevice.get();
	}

	void PresentBegin(CommandList& cmd)
	{
		mImpl->mDevice->PresentBegin(cmd);
	}

	void PresentEnd(CommandList& cmd)
	{
		mImpl->mDevice->PresentEnd(cmd);
	}

	void EndFrame()
	{
		mImpl->mCurrentFrameCount++;
		mImpl->mDevice->EndFrame();
		mImpl->ProcessReleasedHandles();
	}

	bool IsHandleValid(ResHandle handle)
	{
		return mImpl->mHandleAllocator.IsValid(handle);
	}

	bool CreateCommandlist(CommandList& cmd)
	{
		Debug::CheckAssertion(cmd.GetHanlde() == ResHandle::INVALID_HANDLE);

		ResHandle handle = mImpl->AllocHandle(RESOURCETYPE_COMMAND_LIST);
		bool ret = mImpl->CheckHandle(handle, mImpl->mDevice->CreateCommandlist(handle));
		if (ret) {
			cmd.SetHanlde(handle);
		}

		return ret;
	}

	bool CompileCommandList(const CommandList& cmd)
	{
		Debug::CheckAssertion(cmd.GetHanlde() != ResHandle::INVALID_HANDLE);
		return mImpl->mDevice->CompileCommandList(cmd.GetHanlde(), cmd);
	}

	bool SubmitCommandList(const CommandList& cmd)
	{
		Debug::CheckAssertion(cmd.GetHanlde() != ResHandle::INVALID_HANDLE);
		auto handle = cmd.GetHanlde();
		return mImpl->mDevice->SubmitCommandLists(Span<ResHandle>(&handle, 1));
	}

	bool SubmitCommandList(Span<CommandList*> cmds)
	{
		DynamicArray<ResHandle> handles;
		for (int i = 0; i < cmds.length(); i++)
		{
			auto handle = cmds[i]->GetHanlde();
			if (handle != ResHandle::INVALID_HANDLE) {
				handles.push(handle);
			}
		}
		return mImpl->mDevice->SubmitCommandLists(Span<ResHandle>(handles.data(), handles.size()));
	}

	ResHandle CreateTexture(const TextureDesc* desc, const SubresourceData* initialData)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_TEXTURE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateTexture(handle, desc, initialData));
		return handle;
	}

	ResHandle CreateBuffer(const GPUBufferDesc* desc, const SubresourceData* initialData)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_BUFFER);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateBuffer(handle, desc, initialData));
		return handle;
	}

	ResHandle CreateShader(SHADERSTAGES stage, const void* bytecode, size_t length)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_SHADER);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateShader(handle, stage, bytecode, length));
		return handle;
	}

	ResHandle CreateSampler(const SamplerDesc* desc)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_SAMPLER_STATE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreateSamplerState(handle, desc));
		return handle;
	}

	ResHandle CreatePipelineState(const PipelineStateDesc* desc)
	{
		ResHandle handle = mImpl->AllocHandle(ResourceType::RESOURCETYPE_PIPELINE);
		mImpl->CheckHandle(handle, mImpl->mDevice->CreatePipelineState(handle, desc));
		return handle;
	}

	void DestroyResource(ResHandle handle)
	{
		if (handle && IsHandleValid(handle)) {
			mImpl->DestroyHandle(handle);
		}
	}
}
}