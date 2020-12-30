#include "renderer.h"
#include "resource\resourceManager.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	bool mIsInitialize = false;

	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	void Initialize(GPU::GPUSetupParams params)
	{
		if (mIsInitialize) {
			return;
		}

		// initialize gpu 
		GPU::Initialize(params);

		// register render res factories
		Shader::RegisterFactory();

		// test
		auto shader = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/default.esf"));
		ResourceManager::WaitForResource(shader);

		mIsInitialize = true;
	}

	bool IsInitialized()
	{
		return mIsInitialize;
	}

	void Uninitialize()
	{
		if (!mIsInitialize) {
			return;
		}

		// unregister render res factories
		ResourceManager::ProcessReleasedResources();
		Shader::UnregisterFactory();

		// uninitialize gpu
		GPU::Uninitialize();

		mIsInitialize = false;
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
}
}