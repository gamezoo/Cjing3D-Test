#include "widgetLog.h"
#include "editor.h"
#include "imguiRhi\imguiEx.h"

namespace Cjing3D
{
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

				switch (msg.mLevel)
				{
				case LogLevel::LVL_DEV:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					break;
				case LogLevel::LVL_INFO:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
					break;
				case LogLevel::LVL_WARNING:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
					break;
				case LogLevel::LVL_ERROR:
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
					break;
				}

				ImGui::TextUnformatted(msg.mMessage.c_str());
				ImGui::PopStyleColor();
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
}