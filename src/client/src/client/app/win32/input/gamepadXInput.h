#pragma once

#if __has_include("xinput.h")

#include "client\common\common.h"
#include "core\platform\platform.h"
#include "core\input\gamepad.h"

#include <xinput.h>
#pragma comment(lib,"xinput.lib")

namespace Cjing3D::XInput
{
	class GamepadXInput : public Gamepad
	{
	public:
		GamepadXInput();
		~GamepadXInput();

		virtual void Update();
		virtual void SetFeedback(const GamepadFeedback& data, U32 index);

	private:
		XINPUT_STATE mInputStates[4] = {};
	};
}

#endif