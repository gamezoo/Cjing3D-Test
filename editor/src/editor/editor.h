#pragma once

#include "client\app\mainComponent.h"
#include "imguiRhi\pipeline.h"
#include "renderer\renderPath\renderGraphPath3D.h"
#include "core\signal\connection.h"
#include "assetCompiler.h"
#include "editorWidgets.h"
#include "settings.h"

namespace Cjing3D
{
	class Event;
	class EditorWidgetGameView;

	class GameEditorRenderer : public RenderGraphPath2D
	{
	public:
		void Start()override;
		void Stop()override;
		void UpdatePipelines(RenderGraph& renderGraph)override;

	private:
		ImGuiPipeline mImGuiPipeline;
	};

	class GameEditor : public MainComponent
	{
	public:
		GameEditor(const std::shared_ptr<Engine>& engine);
		virtual ~GameEditor();

		void PreInitialize()override;
		void Initialize()override;
		void Uninitialize()override;
		void Update(F32 deltaTime)override;
		void HandleSystemMessage(const Event& systemEvent);
		void RequestExit();

		void LoadEditorSetting();
		void SaveEditorSetting();

		bool IsUniverseChanged()const;

		void RegisterWidget(const char* name, SharedPtr<EditorWidget> widget, bool registerToViewMenu = false);
		SharedPtr<EditorWidget> GetWidget(const StringID& name);
		AssetCompiler& GetAssetCompiler();
		DynamicArray<EditorWidget*>& GetRegisteredMenuViews() { return mRegisteredMenuViews; }

	private:
		void Render()override;
		void DockingBegin();
		void DockingEnd();

	private:
		bool mIsShowDemo = false;
		bool mIsDockingEnable = false;
		bool mIsDockingBegin = false;
		EditorSettings mSettings;

		HashMap<StringID, I32> mWidgetMap;
		DynamicArray<SharedPtr<EditorWidget>> mWidgets;
		DynamicArray<EditorWidget*> mRegisteredMenuViews;

		GameEditorRenderer* mRenderer = nullptr;
		ScopedConnection mEventConnection;
		UniquePtr<AssetCompiler> mAssetCompiler;
	};
}