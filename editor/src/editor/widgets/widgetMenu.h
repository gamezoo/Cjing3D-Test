#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetMenu : public EditorWidget
	{
	public:
		EditorWidgetMenu(GameEditor& editor);

		void Update(F32 deltaTime)override;

	private:
		bool bShowAboutWindow = false;
		void ShowAboutWindow();
	};
}