#pragma once

#ifdef CJING3D_PLATFORM_WIN32
#include <Windows.h>

#if !defined(NOMINMAX) && defined(_MSC_VER)
#define NOMINMAX
#endif
#endif

#include <functional>

namespace Cjing3D
{
	enum ConsoleFontColor
	{
		CONSOLE_FONT_WHITE,
		CONSOLE_FONT_BLUE,
		CONSOLE_FONT_YELLOW,
		CONSOLE_FONT_GREEN,
		CONSOLE_FONT_RED
	};

	void SetLoggerConsoleFontColor(ConsoleFontColor fontColor);
	void ShowMessageBox(const char* msg);
	void LoadFileFromOpenWindow(const char* fileFilter, std::function<void(const char*)> callback);
	void SaveFileToOpenWindow(const char* fileFilter, std::function<void(const char*)> callback);
	void ShowBrowseForFolder(const char* title, std::function<void(const char*)> callback);
}