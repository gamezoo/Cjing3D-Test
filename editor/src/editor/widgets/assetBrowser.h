#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetAssetBrowser : public EditorWidget
	{
	public:
		EditorWidgetAssetBrowser(GameEditor& editor);
		void Update(F32 deltaTime)override;

		void SetCurrentDir(const char* path);

	private:
		void ShowFileList();

		StaticString<128> mFilter;
		MaxPathString mCurrentDir;
		bool mIsDirty = true;
		DynamicArray<MaxPathString> mCurrentSubDirs;

		struct AssetFileInfo
		{
			MaxPathString mFilePath;
		};
		DynamicArray<AssetFileInfo> mAssetFileInfos;
	};
}