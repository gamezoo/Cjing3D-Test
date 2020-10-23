#pragma once

#ifdef CJING3D_PLATFORM_WIN32

#include "client\common\common.h"
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
		GameAppWin32();
		~GameAppWin32() = default;

		void SetInstance(HINSTANCE hInstance);
		void SetAssetPath(const String& path, const String& name);
		void SetTitleName(const UTF8String& string);
		void SetScreenSize(const I32x2& screenSize);

		using CreateGameFunc = std::function<std::unique_ptr<MainComponent>(const SharedPtr<Engine>)>;
		void Run(const CreateGameFunc& createGame);

	private:
		HINSTANCE   mHinstance;
		String		mAssetName = "";
		String		mAssetPath = "Assets";
		UTF8String  mTitleName;
		I32x2       mScreenSize = I32x2(0, 0);
	};
}

#endif
