#pragma once

#include "core\scene\universe.h"
#include "core\engine.h"
#include "renderer\renderer.h"

namespace Cjing3D
{
	struct CameraComponent
	{
		F32 mWidth = 0.0f;
		F32 mHeight = 0.0f;
		F32 mNear = 0.1f;
		F32 mFar = 800.0f;
		F32 mFov = XM_PI / 3.0f;
	};

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

		CullResult GetCullResult();
	};
}