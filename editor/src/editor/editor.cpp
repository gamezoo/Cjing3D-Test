#include "editor.h"
#include "imguiRhi\manager.h"
#include "core\signal\eventQueue.h"
#include "client\app\systemEvent.h"

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

		// modulers
		mAssertCompiler = CJING_MAKE_UNIQUE<AssertCompiler>(*this);

		// load shader lazy after assert compiler initialzed
		//Renderer::LoadAllShaders();

		// imguiRhi
		ImGuiConfigFlags configFlags =
			ImGuiConfigFlags_NavEnableKeyboard |
			ImGuiConfigFlags_DockingEnable |
			ImGuiConfigFlags_ViewportsEnable;
		ImGuiRHI::Manager::Initialize(configFlags);

		mEventConnection = mEngine->GetEventQueue()->Connect([this](const Event& event) {
			HandleSystemMessage(event);
		});

		mRenderer = CJING_NEW(GameEditorRenderer);
		SetRenderPath(mRenderer);

		// widgets
		RegisterWidget("MenuBar", CJING_MAKE_SHARED<EditorWidgetMenu>(*this));

		mAssertCompiler->SetupAsserts();
	}

	void GameEditor::Uninitialize()
	{
		for (auto widget : mWidgets) {
			widget->Uninitialize();
		}
		mWidgets.clear();

		SetRenderPath(nullptr);
		CJING_DELETE(mRenderer);

		mAssertCompiler.Reset();

		ImGuiRHI::Manager::Uninitialize();
		MainComponent::Uninitialize();
	}

	void GameEditor::Update(F32 deltaTime)
	{
		MainComponent::Update(deltaTime);

		/////////////////////////////////////////////////////////////////////////
		// update 
		mAssertCompiler->Update(deltaTime);



		/////////////////////////////////////////////////////////////////////////
		// update gui
		U32x2 resolution = GPU::GetResolution();
		InputManager* input = mEngine->GetInputManager();
		ImGuiRHI::Manager::BeginFrame(*input, (F32)resolution.x(), (F32)resolution.y(), deltaTime);

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			DockingBegin();
		}

		// show custom widgets
		for (auto& widget : mWidgets)
		{
			if (widget != nullptr && widget->Begin())
			{
				widget->Update(deltaTime);
				widget->End();
			}
		}

		if (mIsShowDemo) {
			ImGui::ShowDemoWindow(&mIsShowDemo);
		}

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			DockingEnd();
		}

		ImGuiRHI::Manager::EndFrame();
	}

	void GameEditor::HandleSystemMessage(const Event& systemEvent)
	{
	}

	void GameEditor::RegisterWidget(const StringID& name, SharedPtr<EditorWidget> widget)
	{
		auto index = mWidgetMap.find(name);
		if (index != nullptr) {
			return;
		}

		mWidgetMap.insert(name, mWidgets.size());
		mWidgets.push(widget);
	}

	SharedPtr<EditorWidget> GameEditor::GetWidget(const StringID& name)
	{
		auto index = mWidgetMap.find(name);
		if (index == nullptr) {
			return nullptr;
		}
		return mWidgets[*index];
	}

	void GameEditor::DockingBegin()
	{
		// full screen without window
		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);

		F32 dockingOffsetY = 0.0f;
		dockingOffsetY += GetWidget("MenuBar")->GetHeight();

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + dockingOffsetY));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - dockingOffsetY));
		ImGui::SetNextWindowViewport(viewport->ID);

		static const char* dockingName = "CjingDocking";
		mIsDockingBegin = ImGui::Begin(dockingName, nullptr, windowFlags);
		ImGui::PopStyleVar(3);

		if (mIsDockingBegin)
		{
			ImGuiID mainWindow = ImGui::GetID(dockingName);
			if (!ImGui::DockBuilderGetNode(mainWindow))
			{
				// reset
				ImGui::DockBuilderRemoveNode(mainWindow);
				ImGui::DockBuilderAddNode(mainWindow, ImGuiDockNodeFlags_None);
				ImGui::DockBuilderSetNodeSize(mainWindow, viewport->Size);

				ImGuiID dockMainID = mainWindow;
				// split node
				ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, 0.2f, nullptr, &dockMainID);
				ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, 0.2f, nullptr, &dockMainID);

				// build window
				ImGui::DockBuilderFinish(dockMainID);
			}

			ImGui::DockSpace(mainWindow, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		}
	}

	void GameEditor::DockingEnd()
	{
		if (mIsDockingBegin) {
			ImGui::End();
		}
	}
}