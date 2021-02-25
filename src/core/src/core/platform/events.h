#pragma once

#include "core\string\utf8String.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
	struct WindowEvent
	{
		Platform::WindowType mWindow = Platform::INVALID_WINDOW;
	};

	// window close event
	struct WindowCloseEvent : WindowEvent {
	};

	// view resize event
	struct ViewResizeEvent : WindowEvent
	{
		U32 width = 0;
		U32 height = 0;
	};

	struct WindowMoveEvent : WindowEvent
	{
		I32 mMoveX;
		I32 mMoveY;
	};

	// keyboard input text event
	struct InputTextEvent : WindowEvent
	{
		UTF8String mInputText;
	};
}