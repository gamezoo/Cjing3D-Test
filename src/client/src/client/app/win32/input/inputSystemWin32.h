#pragma once

#include "core\platform\platform.h"
#include "core\input\InputSystem.h"
#include "core\platform\events.h"
#include "client\app\win32\gameWindowWin32.h"

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

		void ProcessTextInput(const InputTextEvent& ent);
		I32 GetTextInput(char* outBuffer, I32 bytes) const override;

		const SharedPtr<KeyBoard> GetKeyBoard()const override;
		const SharedPtr<Mouse>    GetMouse()const override;
		const SharedPtr<Gamepad>  GetGamePad()const override;

	private:
		struct InputManagerWin32Impl* mImpl = nullptr;
	};
}