#pragma once

#include "core\platform\platform.h"
#include "core\input\keyboard.h"

namespace Cjing3D::Win32
{
	class KeyBoardWin32 : public KeyBoard
	{
	public:
		void ProcessKeyboardEvent(const RAWKEYBOARD& keyboard);
	};
}