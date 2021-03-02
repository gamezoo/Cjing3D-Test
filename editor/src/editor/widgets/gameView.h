#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetGameView : public EditorWidget
	{
	public:
		EditorWidgetGameView(GameEditor& editor);

		void Update(F32 deltaTime)override;
	};
}