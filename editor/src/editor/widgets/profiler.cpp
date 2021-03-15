#include "profiler.h"
#include "core\helper\profiler.h"
#include "core\helper\stream.h"

namespace Cjing3D
{
	class EditorWidgetProfilerImpl
	{
	public:
		bool mIsPaused = false;
		MemoryStream mProfileData;

	public:
		void ShowGeneralInfo();
		void ShowCPUProfiler();
		void OnProfilePaused();
	};

	void EditorWidgetProfilerImpl::ShowGeneralInfo()
	{
	}

	void EditorWidgetProfilerImpl::ShowCPUProfiler()
	{
		if (ImGui::Button(mIsPaused ? "Start" : "Pause"))
		{
			mIsPaused = !mIsPaused;
			Profiler::SetPause(mIsPaused);

			if (mIsPaused) {
				OnProfilePaused();
			}
		}
		ImGui::SameLine();
		ImGui::Text("Press \"Start\" to start profiling, press \"Pause\" to show infos");
	}

	void EditorWidgetProfilerImpl::OnProfilePaused()
	{
	}

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
		mImpl->mIsPaused = Profiler::IsPaused();
	}

	void EditorWidgetProfiler::Update(F32 deltaTime)
	{
		if (ImGui::BeginTabBar("profiler_tabs"))
		{
			if (ImGui::BeginTabItem("General"))
			{
				mImpl->ShowGeneralInfo();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("CPU/GPU"))
			{
				mImpl->ShowCPUProfiler();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Memory"))
			{
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Resources"))
			{
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}