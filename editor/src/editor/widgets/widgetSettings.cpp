#include "widgetSettings.h"
#include "editor.h"
#include "imguiRhi\imguiEx.h"

namespace Cjing3D
{
	EditorWidgetSetting::EditorWidgetSetting(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "Setting";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
	}

	void EditorWidgetSetting::Update(F32 deltaTime)
	{
		if (ImGui::Button("Save")) {
			mEditor.SaveEditorSetting();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload")) {

		}

		if (ImGui::BeginTabBar("tabs"))
		{
			if (ImGui::BeginTabItem("General"))
			{
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Style"))
			{
				ImGui::ShowStyleEditor();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
}