#include "renderer.h"
#include "renderScene.h"
#include "renderImage.h"
#include "resource\resourceManager.h"
#include "core\platform\platform.h"
#include "core\scene\universe.h"
#include "core\helper\profiler.h"
#include "core\helper\enumTraits.h"

namespace Cjing3D
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////
	// Impl
	//////////////////////////////////////////////////////////////////////////
	class RendererImpl
	{
	public:
		RendererImpl();
		~RendererImpl();

		void PreLoadShaders();
		void PreLoadBuffers();

	public:
		Concurrency::RWLock mLock;
		RenderScene* mRenderScene = nullptr;

		StaticArray<ShaderRef, SHADERTYPE_COUNT> mShaders;
		StaticArray<GPU::ResHandle, CBTYPE_COUNT> mConstantBuffers;

		DynamicArray<GPU::ResHandle> mSamplerStates;
	};

	RendererImpl::RendererImpl()
	{
	}

	RendererImpl::~RendererImpl()
	{
		for (int i = 0; i < SHADERTYPE_COUNT; i++) {
			mShaders[i].Reset();
		}

		// release gpu handles
		for (const auto& handle : mConstantBuffers)
		{
			if (handle != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(handle);
			}
		}

		for (const auto& handle : mSamplerStates)
		{
			if (handle != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(handle);
			}
		}

		mRenderScene = nullptr;
	}

	void RendererImpl::PreLoadBuffers()
	{
		GPU::BufferDesc desc = {};
		desc.mByteWidth = sizeof(FrameCB);
		desc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
		mConstantBuffers[CBTYPE_FRAME] = GPU::CreateBuffer(&desc, nullptr, "FrameCB");

		desc.mByteWidth = sizeof(CameraCB);
		desc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
		mConstantBuffers[CBTYPE_CAMERA] = GPU::CreateBuffer(&desc, nullptr, "CameraCB");
	}

	void RendererImpl::PreLoadShaders()
	{
		// clear current resource jobs
		ResourceManager::WaitAll();

		mShaders[SHADERTYPE_MAIN]  = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/main_pipeline.jsf"));
		mShaders[SHADERTYPE_IMAGE] = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/render_image.jsf"));

		// wait for all shaders
		ResourceManager::WaitAll();
	}

	//////////////////////////////////////////////////////////////////////////
	// Function
	//////////////////////////////////////////////////////////////////////////
	RendererImpl* mImpl = nullptr;

	void Initialize(GPU::GPUSetupParams params)
	{
		if (IsInitialized()) {
			return;
		}

		// initialize gpu 
		GPU::Initialize(params);

		// register render res factories
		Shader::RegisterFactory();

		// initialize impl
		mImpl = CJING_NEW(RendererImpl);
		mImpl->PreLoadBuffers();
		mImpl->PreLoadShaders();

		// initialize renderImage
		RenderImage::Initialize();
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

		// uninitialize impl
		CJING_SAFE_DELETE(mImpl);

		// unregister render res factories
		ResourceManager::ProcessReleasedResources();
		Shader::UnregisterFactory();

		// uninitialize gpu
		GPU::Uninitialize();
	}

	void InitRenderScene(Engine& engine, Universe& universe)
	{
		UniquePtr<RenderScene> renderScene = CJING_MAKE_UNIQUE<RenderScene>(engine, universe);
		renderScene->Initialize();
		universe.AddScene(std::move(renderScene));
		mImpl->mRenderScene = renderScene.Get();
	}

	void Update(CullResult& visibility, FrameCB& frameCB, F32 deltaTime)
	{
		auto screenSize = GPU::GetScreenSize();
		frameCB.gFrameScreenSize = screenSize;
		frameCB.gFrameScreenSizeRCP = F32x2(1.0f / screenSize[0], 1.0f / screenSize[1]);
	}

	void PresentBegin(GPU::CommandList& cmd)
	{
		GPU::PresentBegin(cmd);
	}

	void PresentEnd()
	{
		GPU::PresentEnd();
	}

	void EndFrame()
	{
		GPU::EndFrame();
	}

	GPU::ResHandle GetConstantBuffer(CBTYPE type)
	{
		return mImpl->mConstantBuffers[(U32)type];
	}

	ShaderRef GetShader(SHADERTYPE type)
	{
		return mImpl->mShaders[(U32)type];
	}

	ShaderRef LoadShader(const char* path, bool waitFor)
	{
		auto shader = ShaderRef(ResourceManager::LoadResource<Shader>(path));
		if (waitFor) {
			ResourceManager::WaitForResource(shader);
		}
		return shader;
	}

	RenderScene* GetRenderScene()
	{
		return mImpl->mRenderScene;
	}

	void AddStaticSampler(const GPU::ResHandle& handle, I32 slot)
	{
		Concurrency::ScopedReadLock lock(mImpl->mLock);
		if (!handle) {
			return;
		}
		mImpl->mSamplerStates.push(handle);
		
		GPU::StaticSampler sam;
		sam.mSampler = handle;
		sam.mSlot = slot;
		GPU::AddStaticSampler(sam);
	}

	void UpdateVisibility(CullResult& visibility, Viewport& viewport, I32 cullingFlag)
	{
		Profiler::BeginCPUBlock("Frustum Culling");



		Profiler::EndCPUBlock();
	}

	void UpdateCameraCB(const Viewport& viewport, CameraCB& cameraCB)
	{
		cameraCB.gCameraVP = XMStore<F32x4x4>(viewport.GetViewProjectionMatrix());
		cameraCB.gCameraView = XMStore<F32x4x4>(viewport.GetViewMatrix());
		cameraCB.gCameraProj = XMStore<F32x4x4>(viewport.GetProjectionMatrix());
		cameraCB.gCameraInvV = XMStore<F32x4x4>(viewport.GetInvViewMatrix());
		cameraCB.gCameraInvP = XMStore<F32x4x4>(viewport.GetInvProjectionMatrix());
		cameraCB.gCameraInvVP = XMStore<F32x4x4>(viewport.GetInvViewProjectionMatrix());

		cameraCB.gCameraPos = viewport.mAt;
		cameraCB.gCameraNearZ = viewport.mNear;
		cameraCB.gCameraFarZ = viewport.mFar;
		cameraCB.gCameraInvNearZ = (1.0f / std::max(0.00001f, cameraCB.gCameraNearZ));
		cameraCB.gCameraInvFarZ = (1.0f / std::max(0.00001f, cameraCB.gCameraFarZ));
	}

}
}