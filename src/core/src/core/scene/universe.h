#pragma once

#include "scene.h"
#include "core\memory\memory.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D
{
	class Universe
	{
	public:
		Universe();
		~Universe();

		DynamicArray<UniquePtr<ECS::IScene>>& GetScenes();
		void AddScene(UniquePtr<ECS::IScene>&& scene);

	private:
		DynamicArray<UniquePtr<ECS::IScene>> mScenes;
	};
}