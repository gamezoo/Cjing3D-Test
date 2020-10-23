#include "inputSystemWin32.h"

namespace Cjing3D::Win32
{
	using namespace XInput;

	InputManagerWin32::InputManagerWin32()
	{
	}

	InputManagerWin32::~InputManagerWin32()
	{
	}

	void InputManagerWin32::Initialize(GameWindowWin32& gameWindow)
	{
		InputManager::Initialize();

		mKeyBoard = CJING_MAKE_SHARED<KeyBoardWin32>();
		mMouse = CJING_MAKE_SHARED<MouseWin32>(gameWindow.GetHwnd());
		mGamepad = CJING_MAKE_SHARED<GamepadXInput>();
	}


	void InputManagerWin32::Uninitialize()
	{
		InputManager::Uninitialize();

		mGamepad = nullptr;
		mKeyBoard = nullptr;
		mMouse = nullptr;
	}

	void InputManagerWin32::ProcessMouseEvent(const RAWMOUSE& mouse)
	{
		mMouse->ProcessMouseEvent(mouse);
	}

	void InputManagerWin32::ProcessKeyboardEvent(const RAWKEYBOARD& keyboard)
	{
		mKeyBoard->ProcessKeyboardEvent(keyboard);
	}
}