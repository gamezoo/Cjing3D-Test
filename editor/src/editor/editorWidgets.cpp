#include "editorWidgets.h"
#include "editor.h"
#include "imguiRhi\imguiEx.h"

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
		if ( mIsBegun || !mIsVisible) {
			return false;
		}

		if (!mIsWindow) {
			return true;
		}
		
		if (mPos[0] != -1.0f && mPos[1] != -1.0f) {
			ImGui::SetNextWindowPos(ImVec2(mPos[0], mPos[1]));
		}
		if (mSize[0] != -1.0f && mSize[1] != -1.0f) {
			ImGui::SetNextWindowSize(ImVec2(mSize[0], mSize[1]));
		}

		mWindow = ImGui::GetCurrentWindow();
		mHeight = ImGui::GetWindowHeight();
		mIsBegun = true;

		if (!PreBegin()) {
			return false;
		}

		if (!ImGui::Begin(mTitleName.c_str(), &mIsVisible, mWidgetFlags)) {
			return false;
		}

		if (!PostBegin()) {
			return false;
		}

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

	EditorWidgetEntityInspector::EditorWidgetEntityInspector(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "Inspector";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
	}

	void EditorWidgetEntityInspector::Update(F32 deltaTime)
	{
	}

	EditorWidgetEntityList::EditorWidgetEntityList(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "Entity List";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
	}

	void EditorWidgetEntityList::Update(F32 deltaTime)
	{
	}
}