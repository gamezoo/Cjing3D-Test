#include "app.h"

namespace Cjing3D
{
	GameApp::GameApp(const std::shared_ptr<Engine>& engine) :
		MainComponent(engine)
	{
	}

	GameApp::~GameApp()
	{
	}

	void GameApp::Initialize()
	{
		MainComponent::Initialize();

		mRenderer = CJING_NEW(RenderGraphPath3D);
		SetRenderPath(mRenderer);
	}

	void GameApp::Uninitialize()
	{
		SetRenderPath(nullptr);
		CJING_DELETE(mRenderer);

		MainComponent::Uninitialize();
	}
}