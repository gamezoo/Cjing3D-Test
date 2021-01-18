#pragma once

#include "core\scene\universe.h"
#include "core\engine.h"

namespace Cjing3D
{
	class RenderScene : public ECS::IScene
	{
	public:
		RenderScene(Engine& engine, Universe& universe);
		virtual ~RenderScene();

		void Initialize() ;
		void Uninitialize();
		void Update(F32 dt);
		void FixedUpdate();
		void Clear();
	};
}