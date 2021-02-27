#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetAssertBrowser : public EditorWidget
	{
	public:
		EditorWidgetAssertBrowser(GameEditor& editor);
		void Update(F32 deltaTime)override;

		void SetCurrentDir(const char* path);

	private:
		void ShowFileList();

		StaticString<128> mFilter;
		MaxPathString mCurrentDir;
		bool mIsDirty = true;
		DynamicArray<MaxPathString> mCurrentSubDirs;

		struct AssertFileInfo
		{
			MaxPathString mFilePath;
		};
		DynamicArray<AssertFileInfo> mAssertFileInfos;
	};
}