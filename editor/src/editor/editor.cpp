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

		mImGuiPipeline.Setup(mMainGraph);
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

	void GameEditor::Update(F32 deltaTime)
	{
		MainComponent::Update(deltaTime);

		U32x2 resolution = GPU::GetResolution();
		InputManager* input = mEngine->GetInputManager();
		ImGuiRHI::Manager::BeginFrame(*input, (F32)resolution.x(), (F32)resolution.y(), deltaTime);

		if (mShowDemo) {
			ImGui::ShowDemoWindow(&mShowDemo);
		}

		ImGuiRHI::Manager::EndFrame();
	}
}