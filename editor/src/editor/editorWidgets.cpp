#include "editorWidgets.h"
#include "editor.h"

namespace Cjing3D
{
	EditorWidget::EditorWidget(GameEditor& editor) :
		mEditor(editor)
	{
	}

	EditorWidget::~EditorWidget()
	{
	}

	bool EditorWidget::Begin()
	{
		if (mIsBegun) {
			return false;
		}

		if (!mIsVisible) {
			return false;
		}
		if (!mIsWindow) {
			return true;
		}


		PreBegin();

		if (mPos[0] != -1.0f && mPos[1] != -1.0f) {
			ImGui::SetNextWindowPos(ImVec2(mPos[0], mPos[1]));
		}

		if (mSize[0] != -1.0f && mSize[1] != -1.0f) {
			ImGui::SetNextWindowSize(ImVec2(mSize[0], mSize[1]));
		}

		if (!ImGui::Begin(mTitleName.c_str(), &mIsVisible, mWidgetFlags)) {
			return false;
		}

		mIsBegun = true;
		mWindow = ImGui::GetCurrentWindow();
		mHeight = ImGui::GetWindowHeight();

		PostBegin();

		return true;
	}

	void EditorWidget::End()
	{
		if (!mIsBegun) {
			return;
		}

		ImGui::End();
		mIsBegun = false;

		ImGui::PopStyleVar(mPushedStyleVar);
		mPushedStyleVar = 0;
	}
	EditorWidgetMenu::EditorWidgetMenu(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "MenuBar";
		mIsWindow = false;
	}

	void EditorWidgetMenu::Update(F32 deltaTime)
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About")) { bShowAboutWindow = true; }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (bShowAboutWindow) {
			ShowAboutWindow();
		}
	}

	void EditorWidgetMenu::ShowAboutWindow()
	{
		ImGui::Begin("About", &bShowAboutWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
		ImGui::Text("Copyright (c) 2020-2021 ZZZY");
		ImGui::Text("GitHub:https://github.com/maoxiezhao/Cjing3D.git");
		ImGui::Text("Cjing3D Engine Version v%s", CjingVersion::GetVersionString());
		ImGui::Text("");
		ImGui::Separator();
		ImGui::Text("License");
		ImGui::Text("MIT License");
		ImGui::Text("Permission is hereby granted, free of charge, to any person obtaining a copy");
		ImGui::Text("of this softwareand associated documentation files(the \"Software\"), to deal");
		ImGui::Text("in the Software without restriction, including without limitation the rights");
		ImGui::Text("to use, copy, modify, merge, publish, distribute, sublicense, and /or sell");
		ImGui::Text("copies of the Software, and to permit persons to whom the Software is");
		ImGui::Text("furnished to do so, subject to the following conditions :");
		ImGui::Text("The above copyright noticeand this permission notice shall be included in all");
		ImGui::Text("copies or substantial portions of the Software.");
		ImGui::Text("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR");
		ImGui::Text("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
		ImGui::Text("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE");
		ImGui::Text("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
		ImGui::Text("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
		ImGui::Text("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE");
		ImGui::Text("SOFTWARE.");
		ImGui::End();
	}
}