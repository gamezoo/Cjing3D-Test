#pragma once

#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\string\string.h"
#include "core\concurrency\concurrency.h"
#include "imguiRhi\manager.h"

namespace Cjing3D
{
	class GameEditor;

	class EditorWidget
	{
	public:
		EditorWidget(GameEditor& editor);
		virtual ~EditorWidget();

		virtual void Initialize() {}
		virtual void Update(F32 deltaTime) {}
		virtual void Uninitialize() {}

		bool Begin();
		void End();

		bool IsVisible()const { return mIsVisible; }
		void SetVisible(bool isVisible) { mIsVisible = isVisible; }
		F32  GetHeight()const { return mHeight; }

	protected:
		virtual bool PreBegin() { return true; }
		virtual bool PostBegin() { return true; }

	protected:
		GameEditor& mEditor;

		bool mIsVisible = true;
		bool mIsBegun = false;
		bool mIsWindow = true;
		I32 mWidgetFlags = 0;
		F32x2 mPos = F32x2(-1.0f, -1.0f);
		F32x2 mSize = F32x2(-1.0f, -1.0f);
		F32 mHeight = 0.0f;
		std::string mTitleName;
		ImGuiWindow* mWindow = nullptr;
		U32 mPushedStyleVar = 0;
	};

	class EditorWidgetMenu : public EditorWidget
	{
	public:
		EditorWidgetMenu(GameEditor& editor);

		void Update(F32 deltaTime)override;

	private:
		bool bShowAboutWindow = false;
		void ShowAboutWindow();
	};

	class EditorWidgetAssertBrowser : public EditorWidget
	{
	public:
		EditorWidgetAssertBrowser(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};

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

	class EditorWidgetSetting : public EditorWidget
	{
	public:
		EditorWidgetSetting(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};

	class EditorWidgetEntityInspector : public EditorWidget
	{
	public:
		EditorWidgetEntityInspector(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};

	class EditorWidgetEntityList : public EditorWidget
	{
	public:
		EditorWidgetEntityList(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};
}