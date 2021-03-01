#include "assetBrowser.h"
#include "assetCompiler.h"
#include "editor.h"
#include "imguiRhi\imguiEx.h"

namespace Cjing3D
{
	EditorWidgetAssetBrowser::EditorWidgetAssetBrowser(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "AssetBrowser";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
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
		ImGui::BeginChild("file_list", ImVec2(120.0f, 0.0f));
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

		U32 dirHash = Path(path).GetHash();
		for (const auto& kvp : resources)
		{
			auto& item = kvp.second;
			if (item.mResDirHash != dirHash) {
				continue;
			}

			auto& assetFileInfo = mAssetFileInfos.emplace();
			assetFileInfo.mFilePath = item.mResPath.c_str();
		}
		// sort by filename
		std::sort(mAssetFileInfos.begin(), mAssetFileInfos.end(),
			[](const AssetFileInfo& a, const AssetFileInfo& b) {
				return strcmp(a.mFilePath.c_str(), b.mFilePath.c_str());
			});
		assetCompiler.UnmapResources();
	}

	void EditorWidgetAssetBrowser::ShowFileList()
	{
	}
}