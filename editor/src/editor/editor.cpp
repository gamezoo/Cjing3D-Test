#include "editor.h"
#include "imguiRhi\manager.h"

namespace Cjing3D
{
	void GameEditorRenderer::Start()
	{
		RenderGraphPath3D::Start();
	}

	void GameEditorRenderer::Stop()
	{
		RenderGraphPath3D::Stop();
	}

	void GameEditorRenderer::UpdatePipelines()
	{
		RenderGraphPath3D::UpdatePipelines();

		mImGuiPipeline.Setup(mMainGraph, mMainGraph.GetResource("rtMain2D"));
	}

	void GameEditorRenderer::Compose(GPU::CommandList& cmd)
	{
	}

	GameEditor::GameEditor(const std::shared_ptr<Engine>& engine) :
		MainComponent(engine)
	{
	
	}

	GameEditor::~GameEditor()
	{
	}

	void GameEditor::Initialize()
	{
		MainComponent::Initialize();

		ImGuiConfigFlags configFlags =
			ImGuiConfigFlags_NavEnableKeyboard |
			ImGuiConfigFlags_DockingEnable |
			ImGuiConfigFlags_ViewportsEnable;
		ImGuiRHI::Manager::Initialize(configFlags);

		mRenderer = CJING_NEW(GameEditorRenderer);
		SetRenderPath(mRenderer);
	}

	void GameEditor::Uninitialize()
	{
		SetRenderPath(nullptr);
		CJING_DELETE(mRenderer);

		ImGuiRHI::Manager::Uninitialize();
		MainComponent::Uninitialize();
	}
}