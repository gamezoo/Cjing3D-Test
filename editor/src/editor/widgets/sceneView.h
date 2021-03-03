#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetSceneView : public EditorWidget
	{
	public:
		EditorWidgetSceneView(GameEditor& editor);
		~EditorWidgetSceneView();

		void Initialize()override;
		void Update(F32 deltaTime)override;
		void Uninitialize()override;

	private:
		class EditorWidgetSceneViewImpl* mImpl = nullptr;
	};
}