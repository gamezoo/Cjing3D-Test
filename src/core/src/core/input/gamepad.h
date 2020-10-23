#pragma once

#include "core\common\common.h"
#include "keyCode.h"

#include "core\container\staticArray.h"

namespace Cjing3D
{
	struct GamepadFeedback
	{
		float leftVibration  = 0;	  
		float rightVibration = 0;
	};

	struct GamepadState
	{
		bool mButtons[Gamepad_Count] = {};
		F32x2 mLeftThumbStick        = F32x2(0.0f, 0.0f);
		F32x2 mRightThumbStick		 = F32x2(0.0f, 0.0f);
		F32 mLeftTrigger			 = 0.0f;
		F32 mRightTrigger			 = 0.0f;
	};

	struct GamepadController
	{
		GamepadState mState;
		bool mIsConnected = false;
	};

	class Gamepad
	{
	public:
		Gamepad() = default;

		virtual void Update() = 0;
		virtual void SetFeedback(const GamepadFeedback& data, U32 index) = 0;

		bool IsConnected(U32 index)const;
		U32 GetMaxGamepadController()const;
		bool IsKeyDown(const KeyCode& key, int index)const;

	protected:
		StaticArray<GamepadController, 4> mGamepadControllers;
	};
}
