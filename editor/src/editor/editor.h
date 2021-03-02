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

	class GameEditorRenderer : public RenderGraphPath2D
	{
	public:
		void Start()override;
		void Stop()override;
		void UpdatePipelines()override;

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

		void RegisterWidget(const StringID& name, SharedPtr<EditorWidget> widget);
		SharedPtr<EditorWidget> GetWidget(const StringID& name);
		AssetCompiler& GetAssetCompiler();

	private:
		void DockingBegin();
		void DockingEnd();

	private:
		bool mIsShowDemo = false;
		bool mIsDockingEnable = false;
		bool mIsDockingBegin = false;
		EditorSettings mSettings;

		HashMap<StringID, I32> mWidgetMap;
		DynamicArray<SharedPtr<EditorWidget>> mWidgets;

		GameEditorRenderer* mRenderer = nullptr;
		ScopedConnection mEventConnection;
		UniquePtr<AssetCompiler> mAssetCompiler;
	};
}