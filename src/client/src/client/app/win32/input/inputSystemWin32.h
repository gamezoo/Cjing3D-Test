#pragma once

#include "core\platform\platform.h"
#include "core\input\InputSystem.h"

#include "client\app\win32\gameWindowWin32.h"
#include "client\app\win32\input\keyboardWin32.h"
#include "client\app\win32\input\mouseWin32.h"
#include "client\app\win32\input\gamepadXInput.h"

namespace Cjing3D::Win32
{
	class InputManagerWin32 : public InputManager
	{
	public:
		InputManagerWin32();
		virtual ~InputManagerWin32();

		void Initialize(GameWindowWin32& gameWindow);
		void Uninitialize()override;

		void ProcessMouseEvent(const RAWMOUSE& mouse);
		void ProcessKeyboardEvent(const RAWKEYBOARD& keyboard);

		const SharedPtr<KeyBoard> GetKeyBoard()const override { return mKeyBoard; }
		const SharedPtr<Mouse>    GetMouse()const override { return mMouse; }
		const SharedPtr<Gamepad>  GetGamePad()const override { return mGamepad;  }

	private:
		SharedPtr<KeyBoardWin32> mKeyBoard = nullptr;
		SharedPtr<MouseWin32> mMouse = nullptr;
		SharedPtr<XInput::GamepadXInput> mGamepad = nullptr;
	};
}