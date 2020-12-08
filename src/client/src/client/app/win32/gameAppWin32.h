#pragma once

#ifdef CJING3D_PLATFORM_WIN32

#include "client\common\common.h"
#include "core\initConfig.h"
#include "core\platform\platform.h"
#include "core\string\utf8String.h"
#include "core\signal\eventQueue.h"

#include <functional>

namespace Cjing3D
{
	class MainComponent;
	class Engine;
}
namespace Cjing3D::Win32
{
	class GameAppWin32
	{
	public:
		GameAppWin32(HINSTANCE hInstance);
		~GameAppWin32() = default;

		using CreateGameFunc = std::function<SharedPtr<MainComponent>(const SharedPtr<Engine>)>;
		void Run(InitConfig config, CreateGameFunc createGame = nullptr);

	private:
		HINSTANCE   mHinstance;
	};
}
#endif
