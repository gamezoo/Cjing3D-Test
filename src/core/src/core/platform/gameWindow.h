#pragma once

#include "core\common\common.h"
#include "math\geometry.h"
#include "core\string\utf8String.h"
#include "core\platform\platform.h"

namespace Cjing3D {

	class GameWindow
	{
	public:
		GameWindow() = default;
		GameWindow(const GameWindow&) = delete;
		GameWindow& operator=(const GameWindow&) = delete;
		virtual ~GameWindow() = default;

		bool IsExiting()const {
			return mIsExiting;
		}
		void SetIsExiting(bool isExiting) {
			mIsExiting = isExiting;
		}

		virtual Platform::WindowType GetWindowHandle() const = 0;
		virtual bool IsWindowActive() const = 0;
		virtual bool IsFullScreen()const = 0;
		virtual UTF8String GetWindowTitle()const = 0;
		virtual void SetWindowTitle(const UTF8String& title) = 0;
		virtual bool IsMouseCursorVisible()const = 0;
		virtual void SetMouseCursorVisible(bool visible) = 0;
		virtual Platform::WindowRect GetClientBounds()const = 0;
		virtual void SetClientbounds(const Platform::WindowRect& rect) = 0;
		virtual I32 GetDPI()const = 0;

	private:
		bool mIsExiting = false;
	};
}