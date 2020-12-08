
#include <windows.h>
#include <iostream>

#include "client\app\win32\gameAppWin32.h"

using namespace Cjing3D;

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	InitConfig config = {};
	config.mScreenSize = { 1280, 720 };
	config.mIsFullScreen = false;
	config.mIsLockFrameRate = true;
	config.mTargetFrameRate = 60;
	config.mTitle = "CJING3D Editor";

	Win32::GameAppWin32 gameApp(hInstance);
	gameApp.Run(config, nullptr);

	return 0;
}
