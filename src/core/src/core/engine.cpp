#include "engine.h"
#include "core\plugin\modulePulgin.h"

namespace Cjing3D
{
	Universe* Engine::CreateUniverse()
	{
		Universe* universe = CJING_NEW(Universe);
		for (auto* modulePlugin : mModulerPlugins) {
			modulePlugin->CreateScene(*universe);
		}

		return universe;
	}

	void Engine::DestroyUniverse(Universe* universe)
	{
		auto& scenes = universe->GetScenes();
		for (auto& scene : scenes)
		{
			scene->Clear();
			scene->Uninitialize();
		}

		CJING_DELETE(universe);
	}

	void Engine::StartUniverse(Universe& universe)
	{
	}

	void Engine::StopUniverse(Universe& universe)
	{
	}
}