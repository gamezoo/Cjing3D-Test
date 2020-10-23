#pragma once

#include "client\common\common.h"
#include "core\string\utf8String.h"

namespace Cjing3D
{
	// window close event
	struct WindowCloseEvent {};

	// view resize event
	struct ViewResizeEvent 
	{
		U32 width = 0;
		U32 height = 0;
	};

	// keyboard input text event
	struct InputTextEvent
	{
		UTF8String mInputText;
	};
}