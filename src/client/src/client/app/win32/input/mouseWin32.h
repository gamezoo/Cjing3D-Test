#pragma once

#include "core\platform\platform.h"
#include "core\input\mouse.h"

namespace Cjing3D::Win32
{
	class MouseWin32 : public Mouse
	{
	public:
		MouseWin32(HWND hWnd);

		void ProcessMouseEvent(const RAWMOUSE& mouse);

	private:
		HWND mHwnd;
	};
}