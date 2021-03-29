#include "assetBrowser.h"
#include "assetCompiler.h"
#include "editor.h"
#include "imguiRhi\imguiEx.h"
#include "resource\resource.h"
#include "resource\converter.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{
	EditorWidgetAssetBrowser::EditorWidgetAssetBrowser(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "AssetBrowser";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
		mAssetInspector = CJING_MAKE_SHARED<EditorWidgetAssetInspector>(editor, *this);

		editor.RegisterWidget("AssetInspector", mAssetInspector);
	}

	EditorWidgetAssetBrowser::~EditorWidgetAssetBrowser()
	{
		mAssetInspector = nullptr;
		UnselectResources();
	}

	void EditorWidgetAssetBrowser::Update(F32 deltaTime)
	{
		if (mIsDirty) {
			SetCurrentDir(mCurrentDir);
		}

		// file filter
		ImGui::PushItemWidth(100);
		if (ImGui::InputTextWithHint("##filter", "Filter", mFilter.c_str(), mFilter.size())) {

		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {

		}
		ImGui::Separator();

		// dir list
		ImGui::BeginChild("dir_list", ImVec2(120.0f, 0.0f));
		ImGui::PushItemWidth(120.0f);
		bool b = false;
		if (ImGui::Selectable("..", &b))
		{
			MaxPathString parentPath;
			Path::GetPathParentPath(mCurrentDir, parentPath.toSpan());
			SetCurrentDir(parentPath.c_str());
		}
		for (const auto& dir : mCurrentSubDirs)
		{
			if (ImGui::Selectable(dir.c_str(), &b))
			{
				MaxPathString rootPath(mCurrentDir, Path::PATH_SEPERATOR, dir);
				SetCurrentDir(rootPath);
			}
		}

		ImGui::EndChild();
		ImGui::SameLine();

		// file list
		ImVec2 leftSize(120.0f, 0.0f);
		ImGuiEx::VSplitter("file_vsplit", leftSize);
		ImGui::SameLine();
		ImGui::BeginChild("file_list");
		ShowFileList();
		ImGui::EndChild();
	}

	void EditorWidgetAssetBrowser::SetCurrentDir(const char* path)
	{
		mIsDirty = false;
		mCurrentSubDirs.clear();
		mAssetFileInfos.clear();

		auto fileSystem = mEditor.GetEngine()->GetFileSystem();
		Path::FormatPath(path, mCurrentDir.toSpan());
		auto dirs = fileSystem->EnumerateFiles(mCurrentDir, EnumrateMode_DIRECTORY);
		for (const auto dir : dirs)
		{
			if (dir[0] != '.') {
				mCurrentSubDirs.push(dir);
			}
		}

		// map compiled resources and push resources from current directory
		auto& assetCompiler = mEditor.GetAssetCompiler();
		auto& resources = assetCompiler.MapResources();

		U32 dirHash = StringUtils::StringToHash(mCurrentDir.c_str());
		for (const auto& kvp : resources)
		{
			auto& item = kvp.second;
			if (item.mResDirHash != dirHash) {
				continue;
			}

			auto& assetFileInfo = mAssetFileInfos.emplace();
			assetFileInfo.mFilePath = item.mResPath.c_str();
			assetFileInfo.mFilePathHash = item.mResPath.GetHash();
		}
		// sort by filename
		std::sort(mAssetFileInfos.begin(), mAssetFileInfos.end(),
			[](const AssetFileInfo& a, const AssetFileInfo& b) {
				return a.mFilePath < b.mFilePath;
			});
		assetCompiler.UnmapResources();
	}

	void EditorWidgetAssetBrowser::ShowFileList()
	{
		I32 w = (I32)ImGui::GetContentRegionAvail().x;
		I32 columns = mShowThumbnails ? (I32)ImGui::GetContentRegionAvail().x / THUMBNAIL_TILE_SIZE : 1;
		I32 count = mAssetFileInfos.size();
		I32 rows = mShowThumbnails ? (count + columns - 1) / columns : count;

		// show file list
		ImGuiListClipper clipper;
		clipper.Begin(rows);
		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
			{
				AssetFileInfo& info = mAssetFileInfos[row];
				bool selected = mSelecetedResSet.find(info.mFilePathHash) != nullptr;
				ImGui::Selectable(info.mFilePath.c_str(), selected);

				// selectable operations
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("%s", info.mFilePath.c_str());
				}

				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsMouseReleased(0)) {
						SelectResource(Path(info.mFilePath));
					}
					else if (ImGui::IsMouseReleased(1)) {
						ImGui::OpenPopup("res_item_popup");
					}
				}
			}
		}

		// show popup context
		auto ShowCommonPopup = [&]() {
			if (ImGui::BeginMenu("Create directory"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("View in explorer"))
			{
				auto* fileSystem = mEditor.GetEngine()->GetFileSystem();
				MaxPathString fullPath(fileSystem->GetBasePath(), Path::PATH_SEPERATOR, mCurrentDir);
				Platform::OpenExplorer(fullPath.c_str());
			}
		};

		if (ImGui::BeginPopup("res_item_popup"))
		{
			if (ImGui::BeginMenu("Rename"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Delete"))
			{
			}
			ImGui::Separator();
			ShowCommonPopup();
			ImGui::EndPopup();
		}
		else if (ImGui::BeginPopupContextWindow("context")) {
			ShowCommonPopup();
			ImGui::EndPopup();
		}
	}

	void EditorWidgetAssetBrowser::SelectResource(const Path& path)
	{
		ResourceType type = ResourceManager::GetResourceType(path.c_str());
		Resource* res = ResourceManager::LoadResource(type, path);
		if (res != nullptr)
		{
			UnselectResources();
			mSelectedResources.push(res);
			mSelecetedResSet.insert(path.GetHash());
			mAssetInspector->RequestFocus();
		}
	}

	void EditorWidgetAssetBrowser::UnselectResources()
	{
		if (mSelectedResources.empty()) {
			return;
		}

		for (auto res : mSelectedResources) {
			ResourceManager::ReleaseResource(&res);
		}
		mSelectedResources.clear();
		mSelecetedResSet.clear();
	}

	EditorWidgetAssetInspector::EditorWidgetAssetInspector(GameEditor& editor, EditorWidgetAssetBrowser& browser) :
		EditorWidget(editor),
		mBrowser(browser)
	{
		mTitleName = "Asset inspector";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar;
	}

	EditorWidgetAssetInspector::~EditorWidgetAssetInspector()
	{
	}

	void EditorWidgetAssetInspector::Initialize()
	{
		auto fileSystem = mEditor.GetEngine()->GetFileSystem();
		mResContext = CJING_MAKE_UNIQUE<ResConverterContext>(*fileSystem);
	}

	void EditorWidgetAssetInspector::Uninitialize()
	{
		mResContext.Reset();
	}

	void EditorWidgetAssetInspector::Update(F32 deltaTime)
	{
		auto& selectedResources = mBrowser.mSelectedResources;
		if (selectedResources.empty()) {
			return;
		}

		if (selectedResources.size() == 1)
		{
			auto& res = selectedResources[0];
			if (res->GetPath() != mResContext->GetSrcPath().c_str()) {
				mResContext->Load(res->GetPath().c_str());
			}

			ImGui::Text("Selected resource");
			ImGui::Separator();
			ImGui::TextUnformatted(res->GetPath().c_str());
			ImGui::Separator();

			ImGuiEx::VLeftLabel("Status");
			if (res->IsFaild()) {
				ImGui::Text("Failed");
			}
			else if (res->IsEmpty()) {
				ImGui::Text("Empty");
			}
			else	
			{
				if (res->IsLoaded()) {
					ImGui::Text("Loaded");
				}
				else {
					ImGui::Text("Not load");
				}
			}

			if (res->IsLoaded())
			{
				ImGuiEx::VLeftLabel("Compiled size");
				ImGui::Text("%.2f KB", res->GetCompiledSize() / 1024.0f);
			}

			ImGui::Separator();
			// specific asset res editor
			MaxPathString ext;
			Path::GetPathExtension(res->GetPath().toSpan(), ext.toSpan());
			auto fileSystem = mEditor.GetEngine()->GetFileSystem();
			auto& plugins = ResourceManager::GetPlugins();
			for (auto plugin : plugins)
			{
				auto converter = plugin->CreateConverter();
				if (converter && converter->SupportsType(ext.c_str(), res->GetType())) {
					converter->OnEditorGUI(*mResContext, res->GetType(), res);
				}
				plugin->DestroyConverter(converter);
			}
		}
	}

	bool EditorWidgetAssetInspector::PreBegin()
	{
		SetVisible(mBrowser.IsVisible());
		return true;
	}

	bool EditorWidgetAssetInspector::PostBegin()
	{
		mBrowser.SetVisible(mIsVisible);
		return true;
	}
}