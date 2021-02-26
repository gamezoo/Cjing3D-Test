#include "editor.h"
#include "imguiRhi\manager.h"
#include "core\signal\eventQueue.h"
#include "core\platform\events.h"
#include "core\serialization\jsonArchive.h"

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

	void GameEditor::PreInitialize()
	{
		RegisterWidget("MenuBar", CJING_MAKE_SHARED<EditorWidgetMenu>(*this));
		RegisterWidget("AssertBrowser", CJING_MAKE_SHARED<EditorWidgetAssertBrowser>(*this));
		RegisterWidget("Log", CJING_MAKE_SHARED<EditorWidgetLog>(*this));
		RegisterWidget("Setting", CJING_MAKE_SHARED<EditorWidgetSetting>(*this));
		RegisterWidget("Inspector", CJING_MAKE_SHARED<EditorWidgetEntityInspector>(*this));
		RegisterWidget("EntityList", CJING_MAKE_SHARED<EditorWidgetEntityList>(*this));
	}

	void GameEditor::Initialize()
	{
		MainComponent::Initialize();

		// creat imgui context
		ImGuiRHI::Manager::CreateContext();

		// load editor settings
		LoadEditorSetting();

		// modulers
		mAssertCompiler = CJING_MAKE_UNIQUE<AssertCompiler>(*this);

		// load shader lazy after assert compiler initialzed
		Renderer::LoadAllShaders();

		// imgui
		ImGuiConfigFlags configFlags =
			ImGuiConfigFlags_NavEnableKeyboard |
			ImGuiConfigFlags_DockingEnable |
			ImGuiConfigFlags_ViewportsEnable;
		ImGuiRHI::Manager::Initialize(configFlags, mEngine->GetGameWindow()->GetWindowHandle());
		
		// load inifile from mem
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		ImGui::LoadIniSettingsFromMemory(mSettings.mImGuiIni.c_str());

		mEventConnection = mEngine->GetEventQueue()->Connect([this](const Event& event) {
			HandleSystemMessage(event);
		});

		// set render path
		mRenderer = CJING_NEW(GameEditorRenderer);
		SetRenderPath(mRenderer);

		// init widgets
		for (auto widget : mWidgets) {
			widget->Initialize();
		}

		// setup asserts
		mAssertCompiler->SetupAsserts();
	}

	void GameEditor::Uninitialize()
	{
		SaveEditorSetting();

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
			if (widget != nullptr)
			{
				if (widget->Begin()) {
					widget->Update(deltaTime);
				}
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

	void GameEditor::RequestExit()
	{
		if (!IsUniverseChanged()) 
		{
			GetEngine()->RequestExit();
			return;
		}
	}

	void GameEditor::LoadEditorSetting()
	{
		Logger::Info("Loading editor settings...");

		JsonArchive archive(ArchiveMode::ArchiveMode_Read, mEngine->GetFileSystem());
		if (!mSettings.Load(archive)) {
			Debug::Die("Failed to load editor settings");
		}
	}

	void GameEditor::SaveEditorSetting()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantSaveIniSettings) {
			mSettings.mImGuiIni = ImGui::SaveIniSettingsToMemory();
		}

		JsonArchive archive(ArchiveMode::ArchiveMode_Write, mEngine->GetFileSystem());
		mSettings.Save(archive);
	}

	bool GameEditor::IsUniverseChanged() const
	{
		return false;
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

	AssertCompiler& GameEditor::GetAssertCompiler()
	{
		return *mAssertCompiler;
	}

	void GameEditor::DockingBegin()
	{
		// full screen without window
		ImGuiWindowFlags flags =
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

		bool viewportsEnable = ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable;

		F32 dockingOffsetY = 0.0f;
		dockingOffsetY += GetWidget("MenuBar")->GetHeight();

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + dockingOffsetY));
		ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - dockingOffsetY));
		ImGui::SetNextWindowViewport(viewport->ID);

		static const char* dockingName = "MainDocking";
		mIsDockingBegin = ImGui::Begin(dockingName, nullptr, flags);
		ImGui::PopStyleVar(3);

		if (mIsDockingBegin)
		{
			ImGuiID mainWindow = ImGui::GetID(dockingName);
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