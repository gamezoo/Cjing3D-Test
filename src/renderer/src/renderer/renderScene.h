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

		void Initialize()override;
		void Uninitialize()override;
		void Update(F32 dt)override;
		void LateUpdate(F32 dt)override;
		void Clear()override;
		Universe& GetUniverse()override;

		CullResult GetCullResult();

		static void RegisterReflect();

	private:
		class RenderSceneImpl* mImpl = nullptr;
	};
}