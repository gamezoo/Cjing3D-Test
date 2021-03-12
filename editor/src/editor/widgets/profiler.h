#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetProfiler : public EditorWidget
	{
	public:
		EditorWidgetProfiler(GameEditor& editor);
		~EditorWidgetProfiler();

		void Initialize()override;
		void Update(F32 deltaTime)override;
		void Uninitialize()override;

	private:
		class EditorWidgetProfilerImpl* mImpl = nullptr;
	};
}