#pragma once

#include "editorWidgets.h"
#include "core\container\set.h"

namespace Cjing3D
{
	class Resource;
	class EditorWidgetAssetInspector;

	class EditorWidgetAssetBrowser : public EditorWidget
	{
	public:
		EditorWidgetAssetBrowser(GameEditor& editor);
		~EditorWidgetAssetBrowser();

		void Update(F32 deltaTime)override;
		void SetCurrentDir(const char* path);

	private:
		friend class EditorWidgetAssetInspector;

		void ShowFileList();
		void SelectResource(const Path& path);
		void UnselectResources();

		StaticString<128> mFilter;
		MaxPathString mCurrentDir;
		bool mIsDirty = true;
		DynamicArray<MaxPathString> mCurrentSubDirs;

		struct AssetFileInfo
		{
			MaxPathString mFilePath;
			U32 mFilePathHash;
		};
		DynamicArray<AssetFileInfo> mAssetFileInfos;

		static const I32 THUMBNAIL_TILE_SIZE = 72;
		bool mShowThumbnails = false;
		DynamicArray<Resource*> mSelectedResources;
		Set<U32> mSelecetedResSet;

		SharedPtr<EditorWidgetAssetInspector> mAssetInspector;
	};

	class EditorWidgetAssetInspector : public EditorWidget
	{
	public:
		EditorWidgetAssetInspector(GameEditor& editor, EditorWidgetAssetBrowser& browser);
		~EditorWidgetAssetInspector();

		void Update(F32 deltaTime)override;

	protected:
		bool PreBegin()override;
		bool PostBegin()override;

	private:
		EditorWidgetAssetBrowser& mBrowser;
	};
}