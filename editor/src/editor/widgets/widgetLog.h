#pragma once

#include "editorWidgets.h"

namespace Cjing3D
{
	class EditorWidgetLog : public EditorWidget
	{
	public:
		EditorWidgetLog(GameEditor& editor);
		virtual ~EditorWidgetLog();

		void Update(F32 deltaTime)override;
		void PushLog(LogLevel level, const char* msg);

	private:
		class UILoggerSink : public LoggerSink
		{
		public:
			UILoggerSink(EditorWidgetLog& logUI) : mLogUI(logUI) {}
			void Log(LogLevel level, const char* msg)override
			{
				mLogUI.PushLog(level, msg);
			}
		private:
			EditorWidgetLog& mLogUI;
		};
		UILoggerSink mLoggerSink;

		struct LogMessage
		{
			String mMessage;
			LogLevel mLevel;
		};
		DynamicArray<LogMessage> mMessages;

		bool mAutoscroll = true;
		bool mScrollToBottom = false;
		I32 mMsgCount[(I32)LogLevel::COUNT];
		I32 mLogLvlMask = 0;
		Concurrency::Mutex mMutex;
	};
}