#pragma once

#ifdef CJING3D_PLATFORM_WIN32
#include <Windows.h>

#if !defined(NOMINMAX) && defined(_MSC_VER)
#define NOMINMAX
#endif
#endif

#include "core\common\definitions.h"
#include "core\container\span.h"

#include <functional>

namespace Cjing3D {
namespace Platform {

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

	/////////////////////////////////////////////////////////////////////////////////
	// File 
	bool   FileExists(const char* path);
	bool   DirExists(const char* path);
	bool   DeleteFile(const char* path);
	bool   MoveFile(const char* from, const char* to);
	bool   FileCopy(const char* from, const char* to);
	size_t GetFileSize(const char* path);
	U64    GetLastModified(const char* file);
	bool   CreateDir(const char* path);
	void   SetCurrentDir(const char* path);
	void   GetCurrentDir(Span<char> path);
}
}