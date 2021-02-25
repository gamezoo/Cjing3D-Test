#pragma once

#ifdef CJING3D_PLATFORM_WIN32

#include "core\platform\platform.h"
#include "core\signal\eventQueue.h"
#include "core\platform\gameWindow.h"
#include "core\container\dynamicArray.h"

#include <functional>

#define DEFAULT_GAME_WINDOW_WIDTH  1440
#define DEFAULT_GAME_WINDOW_HEIGHT 800
#define DEFAULT_GAME_WINDOW_DPI    96

namespace Cjing3D {
	struct InitConfig;
}

namespace Cjing3D::Win32 {

	struct WindowMessageData
	{
		HWND handle;
		uint32_t message;
		uint64_t wparam;
		int64_t lparam;
	};
	using WindowMessageHandler = std::function<bool(const WindowMessageData& window)>;

	class GameWindowWin32 : public GameWindow
	{
	public:
		GameWindowWin32(HINSTANCE hInstance, const UTF8String& titleName, const SharedPtr<EventQueue>& eventQueue, const InitConfig& config);
		~GameWindowWin32();

		bool Tick();
		void ShutDown();
		bool HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT&result);
		void AddMessageHandler(WindowMessageHandler handler);

		Platform::WindowType GetWindowHandle() const override;
		bool IsWindowActive() const override;
		bool IsFullScreen()const override;
		UTF8String GetWindowTitle()const override;
		void SetWindowTitle(const UTF8String& title) override;
		bool IsMouseCursorVisible()const override;
		void SetMouseCursorVisible(bool visible) override;
		RectInt GetClientBounds()const override;
		void SetClientbounds(const RectInt& rect) override;
		I32 GetDPI()const override;

		HINSTANCE GetInstance()const { return mHinstance; }
		HWND GetHwnd()const { return mHwnd; }

	private:
		/** Wnd Proc Function */
		static GameWindowWin32* GetWindowCaller(HWND, UINT, WPARAM, LPARAM);
		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	private:
		DynamicArray<WindowMessageHandler> mHandlers;
		I32 mDPI = DEFAULT_GAME_WINDOW_DPI;
		UTF8String mTitleName;
		I32x2 mScreenPos = I32x2(0, 0);
		I32x2 mScreenSize = I32x2(0, 0);
		bool mIsMouseCursorVisible = true;
		bool mIsFullScreen = false;
		bool mIsInitialized = false;
		HINSTANCE mHinstance;
		HWND mHwnd;
		SharedPtr<EventQueue> mEventQueue = nullptr;
	};
}

#endif