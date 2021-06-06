#pragma once

#include "client\app\mainComponent.h"
#include "renderer\renderPath\renderGraphPath3D.h"

namespace Cjing3D
{
	class GameApp : public MainComponent
	{
	public:
		GameApp(const std::shared_ptr<Engine>& engine);
		virtual ~GameApp();

		void Initialize()override;
		void Uninitialize()override;

	private:
		RenderGraphPath3D* mRenderer = nullptr;
	};
}