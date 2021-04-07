#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetGameView : public EditorWidget
	{
	public:
		EditorWidgetGameView(GameEditor& editor);
		~EditorWidgetGameView();

		void Initialize()override;
		void Update(F32 deltaTime)override;
		void Draw()override;
		void Uninitialize()override;

	private:
		class EditorWidgetGameViewImpl* mImpl = nullptr;
	};
}