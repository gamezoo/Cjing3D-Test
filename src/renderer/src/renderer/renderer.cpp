#include "renderer.h"
#include "renderScene.h"
#include "resource\resourceManager.h"
#include "core\platform\platform.h"
#include "core\scene\universe.h"

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

	private:
		StaticArray<ShaderRef, SHADERTYPE_COUNT> mShaders;
	};

	RendererImpl::RendererImpl()
	{
	}

	RendererImpl::~RendererImpl()
	{
		for (int i = 0; i < SHADERTYPE_COUNT; i++) {
			mShaders[i].Reset();
		}
	}

	void RendererImpl::PreLoadShaders()
	{
		// clear current resource jobs
		ResourceManager::WaitAll();

		mShaders[SHADERTYPE::SHADERTYPE_DEFAULT] = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/default_pipeline.jsf"));

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
		return ShaderRef();
	}

	ShaderRef LoadShader(const char* path, bool waitFor)
	{
		auto shader = ShaderRef(ResourceManager::LoadResource<Shader>(path));
		if (waitFor) {
			ResourceManager::WaitForResource(shader);
		}
		return shader;
	}

}
}