#include "profiler.h"

namespace Cjing3D
{
	class EditorWidgetProfilerImpl
	{
	public:
	};

	EditorWidgetProfiler::EditorWidgetProfiler(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "Profiler";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavInputs;
		mImpl = CJING_NEW(EditorWidgetProfilerImpl);
	}

	EditorWidgetProfiler::~EditorWidgetProfiler()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void EditorWidgetProfiler::Initialize()
	{
	}

	void EditorWidgetProfiler::Update(F32 deltaTime)
	{
	}

	void EditorWidgetProfiler::Uninitialize()
	{
	}
}