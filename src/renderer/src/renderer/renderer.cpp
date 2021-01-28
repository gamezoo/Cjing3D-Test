#include "renderer.h"
#include "renderScene.h"
#include "renderImage.h"
#include "resource\resourceManager.h"
#include "core\platform\platform.h"
#include "core\scene\universe.h"

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

	public:
		Concurrency::RWLock mLock;
		StaticArray<ShaderRef, SHADERTYPE_COUNT> mShaders;
		RenderScene* mRenderScene = nullptr;

		// samplerState
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

		// release sampler states
		for (auto samplerState : mSamplerStates) 
		{
			if (samplerState != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(samplerState);
			}
		}

		mRenderScene = nullptr;
	}

	void RendererImpl::PreLoadShaders()
	{
		// clear current resource jobs
		ResourceManager::WaitAll();

		mShaders[SHADERTYPE::SHADERTYPE_MAIN]  = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/main_pipeline.jsf"));
		mShaders[SHADERTYPE::SHADERTYPE_IMAGE] = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/render_image.jsf"));

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

	void Update(CullResult& visibility, F32 deltaTime)
	{
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

	}

}
}