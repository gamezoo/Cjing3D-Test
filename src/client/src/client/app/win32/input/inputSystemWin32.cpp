#include "inputSystemWin32.h"
#include "client\app\win32\input\keyboardWin32.h"
#include "client\app\win32\input\mouseWin32.h"
#include "client\app\win32\input\gamepadXInput.h"

namespace Cjing3D::Win32
{
	using namespace XInput;

	struct InputManagerWin32Impl
	{
		SharedPtr<KeyBoardWin32> mKeyBoard = nullptr;
		SharedPtr<MouseWin32> mMouse = nullptr;
		SharedPtr<XInput::GamepadXInput> mGamepad = nullptr;
		UTF8String mInputText;
	};

	InputManagerWin32::InputManagerWin32()
	{
		mImpl = CJING_NEW(InputManagerWin32Impl);
	}

	InputManagerWin32::~InputManagerWin32()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void InputManagerWin32::Initialize(GameWindowWin32& gameWindow)
	{
		InputManager::Initialize();

		mImpl->mKeyBoard = CJING_MAKE_SHARED<KeyBoardWin32>();
		mImpl->mMouse = CJING_MAKE_SHARED<MouseWin32>(gameWindow.GetHwnd());
		mImpl->mGamepad = CJING_MAKE_SHARED<GamepadXInput>();
	}


	void InputManagerWin32::Uninitialize()
	{
		InputManager::Uninitialize();

		mImpl->mGamepad = nullptr;
		mImpl->mKeyBoard = nullptr;
		mImpl->mMouse = nullptr;
	}

	void InputManagerWin32::ProcessMouseEvent(const RAWMOUSE& mouse)
	{
		mImpl->mMouse->ProcessMouseEvent(mouse);
	}

	void InputManagerWin32::ProcessKeyboardEvent(const RAWKEYBOARD& keyboard)
	{
		mImpl->mKeyBoard->ProcessKeyboardEvent(keyboard);
	}

	void InputManagerWin32::ProcessTextInput(const InputTextEvent& ent)
	{
		UTF8String& inputText = mImpl->mInputText;
		inputText.clear();
		inputText.append(ent.mInputText);
	}

	I32 InputManagerWin32::GetTextInput(char* outBuffer, I32 bytes) const
	{
		I32 size = mImpl->mInputText.size();
		if (outBuffer == nullptr) {
			return size;
		}

		if (size <= 0) {
			return 0;
		}

		size = std::min(bytes, size);
		CopyString(Span(outBuffer, size), mImpl->mInputText.c_str());
		return size;
	}

	const SharedPtr<KeyBoard> InputManagerWin32::GetKeyBoard() const
	{
		return mImpl->mKeyBoard;
	}

	const SharedPtr<Mouse> InputManagerWin32::GetMouse() const
	{
		return mImpl->mMouse;
	}

	const SharedPtr<Gamepad> InputManagerWin32::GetGamePad() const
	{
		return mImpl->mGamepad;
	}
}