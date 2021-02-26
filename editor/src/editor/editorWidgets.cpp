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
				if(ImGui::MenuItem("Exit")) {
					mEditor.RequestExit();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Entity"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				if(ImGui::MenuItem("Assert browser", nullptr, mEditor.GetWidget("AssertBrowser")->IsVisible())) {
					mEditor.GetWidget("AssertBrowser")->SetVisible(true);
				};
				if (ImGui::MenuItem("Log", nullptr, mEditor.GetWidget("Log")->IsVisible())) {
					mEditor.GetWidget("Log")->SetVisible(true);
				};
				if (ImGui::MenuItem("Setting", nullptr, mEditor.GetWidget("Setting")->IsVisible())) {
					mEditor.GetWidget("Setting")->SetVisible(true);
				};
				if (ImGui::MenuItem("Entity List", nullptr, mEditor.GetWidget("EntityList")->IsVisible())) {
					mEditor.GetWidget("EntityList")->SetVisible(true);
				};
				if (ImGui::MenuItem("Entity Inspector", nullptr, mEditor.GetWidget("Inspector")->IsVisible())) {
					mEditor.GetWidget("Inspector")->SetVisible(true);
				};
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

	EditorWidgetAssertBrowser::EditorWidgetAssertBrowser(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "AssertBrowser";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
	}

	void EditorWidgetAssertBrowser::Update(F32 deltaTime)
	{
	}
	
	EditorWidgetLog::EditorWidgetLog(GameEditor& editor) :
		EditorWidget(editor),
		mLoggerSink(*this)
	{
		mTitleName = "Log";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse;
		mLogLvlMask = 1 | 2 | 4 | 8;

		for (I32 i = 0; i < (I32)LogLevel::COUNT; i++) {
			mMsgCount[i] = 0;
		}

		Logger::RegisterSink(mLoggerSink);
	}

	EditorWidgetLog::~EditorWidgetLog()
	{
		Logger::UnregisterSink(mLoggerSink);
	}

	void EditorWidgetLog::Update(F32 deltaTime)
	{
		// log level checkbox
		const char* labels[] = { "Dev", "Info", "Warning", "Error" };
		for (I32 i = 0; i < ARRAYSIZE(labels); i++)
		{
			StaticString<64> label;
			label.Sprintf("%s(%d)", labels[i], mMsgCount[i]);
			if (i > 0) {
				ImGui::SameLine();
			}

			bool selected = mLogLvlMask & (1 << i);
			if (ImGui::Checkbox(label.c_str(), &selected)) 
			{
				mLogLvlMask = selected ? mLogLvlMask | (1 << i) : mLogLvlMask & ~(1 << i);
				mMsgCount[i] = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Checkbox("Autoscroll", &mAutoscroll);

		// log messages
		if (ImGui::BeginChild("logMessages", ImVec2(0, 0), true))
		{
			for (const auto& msg : mMessages)
			{
				if ((mLogLvlMask & (1 << (I32)msg.mLevel)) == 0) {
					continue;
				}

				ImGui::TextUnformatted(msg.mMessage.c_str());
			}

			if (mScrollToBottom) {
				mScrollToBottom = false;
				ImGui::SetScrollHereY();
			}
		}
		ImGui::EndChild();

		// popup content
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
			ImGui::OpenPopup("Context");
		}
		if (ImGui::BeginPopup("Context"))
		{
			if (ImGui::Selectable("Copy"))
			{
				String totalMsg;
				for (const auto& logMsg : mMessages)
				{
					totalMsg += logMsg.mMessage;
					totalMsg += "\n";
				}
				if (!totalMsg.empty()) {
					Platform::CopyToClipBoard(totalMsg.c_str());
				}
			}
			if (ImGui::Selectable("Clear"))
			{
				for (I32 i = 0; i < (I32)LogLevel::COUNT; i++)
				{
					if ((mLogLvlMask & (1 << i)) != 0) {
						mMsgCount[i] = 0;
					}
				}

				DynamicArray<LogMessage> newMessages;
				for (const auto& logMsg : mMessages)
				{
					if ((mLogLvlMask & (1 << (I32)logMsg.mLevel)) == 0) {
						newMessages.push(logMsg);
					}
				}
				mMessages = newMessages;
			}
			ImGui::EndPopup();
		}
	}

	void EditorWidgetLog::PushLog(LogLevel level, const char* msg)
	{
		Concurrency::ScopedMutex lock(mMutex);
		mMsgCount[(I32)level]++;

		auto& logMsg = mMessages.emplace();
		logMsg.mLevel = level;
		logMsg.mMessage = msg;

		if (mAutoscroll) {
			mScrollToBottom = true;
		}
	}

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