#include "universe.h"

namespace Cjing3D
{
	Universe::Universe()
	{
	}

	Universe::~Universe()
	{

	}

	DynamicArray<UniquePtr<ECS::IScene>>& Universe::GetScenes()
	{
		return mScenes;
	}

	void Universe::AddScene(UniquePtr<ECS::IScene>&& scene)
	{
	}
}