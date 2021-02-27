#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetSetting : public EditorWidget
	{
	public:
		EditorWidgetSetting(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};
}