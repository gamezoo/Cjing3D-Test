#pragma once

#include "core\common\common.h"
#include "core\input\keyboard.h"
#include "core\input\mouse.h"
#include "core\input\gamepad.h"

#include "core\container\map.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D
{
class InputManager
{
public:
	InputManager();
	virtual ~InputManager();

	virtual void Initialize();
	virtual void Update(F32 deltaTime);
	virtual void Uninitialize();

	// keyboard
	bool IsKeyDown(const KeyCode& key, U32 index = 0)const;
	bool IsKeyPressed(const KeyCode& key, U32 index = 0);
	bool IsKeyReleased(const KeyCode& key, U32 index = 0);
	bool IsKeyHold(const KeyCode& key, U32 index = 0, U32 frames = 16, bool continuous = true);

	// mouse
	I32x2 GetMousePos()const;
	F32 GetMouseWheelDelta()const;
	MouseState GetMouseState()const;
	MouseState GetPeveMouseState()const;

	// text input
	virtual I32 GetTextInput(char* outBuffer, I32 bytes) const { return 0; }

	virtual const SharedPtr<KeyBoard> GetKeyBoard()const = 0;
	virtual const SharedPtr<Mouse>    GetMouse()const = 0;
	virtual const SharedPtr<Gamepad>  GetGamePad()const = 0;

private:
	struct InputInst
	{
		KeyCode mKeyCode = KeyCode::Unknown;
		U32 mControllerIndex = 0;

		bool operator<(const InputInst& rhs) {
			return (mKeyCode != rhs.mKeyCode || mControllerIndex != rhs.mControllerIndex);
		}

		struct LessComparer {
			bool operator()(const InputInst& a, const InputInst& b) const {
				return (a.mKeyCode < b.mKeyCode || a.mControllerIndex < b.mControllerIndex);
			}
		};
	};
	Map<InputInst, U32, InputInst::LessComparer> mInputKeys;

	struct InputController
	{
		enum ControllerType
		{
			UNKNOWN,
			GAMEPAD,
			RAWINPUT,
		};
		ControllerType mType = UNKNOWN;
		int mIndex = 0;
	};
	DynamicArray<InputController> mInputControllers;

	void RegisterController(U32 index, InputController::ControllerType type);
};
}