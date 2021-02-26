#pragma once

#ifdef CJING3D_PLATFORM_WIN32
#include <Windows.h>

#if !defined(NOMINMAX) && defined(_MSC_VER)
#define NOMINMAX
#endif
#endif

#include "core\common\definitions.h"
#include "core\container\span.h"
#include "core\filesystem\file.h"
#include "core\signal\event.h"
#include "core\signal\connection.h"

#include <functional>

namespace Cjing3D {
namespace Platform {

#ifdef CJING3D_PLATFORM_WIN32
	using WindowType = HWND;
	static const HWND INVALID_WINDOW = NULL;
#else 
	using WindowType = int;
	static const int INVALID_WINDOW = 0;
#endif 

	struct WindowRect
	{
		I32 mLeft = 0;
		I32 mTop = 0;
		I32 mRight = 0;
		I32 mBottom = 0;
	};

	struct WindowPoint
	{
		I32 mPosX = 0;
		I32 mPosY = 0;
	};

	enum CursorType
	{
		DEFAULT,
		SIZE_ALL,
		SIZE_NS,
		SIZE_WE,
		SIZE_NWSE,
		LOAD,
		TEXT_INPUT,

		UNDEFINED
	};

	enum ConsoleFontColor
	{
		CONSOLE_FONT_WHITE,
		CONSOLE_FONT_BLUE,
		CONSOLE_FONT_YELLOW,
		CONSOLE_FONT_GREEN,
		CONSOLE_FONT_RED
	};

	void Initialize();

	void SetLoggerConsoleFontColor(ConsoleFontColor fontColor);
	void ShowMessageBox(const char* msg);
	void LoadFileFromOpenWindow(const char* fileFilter, std::function<void(const char*)> callback);
	void SaveFileToOpenWindow(const char* fileFilter, std::function<void(const char*)> callback);
	void ShowBrowseForFolder(const char* title, std::function<void(const char*)> callback);
	bool ShellExecuteOpen(const char* path, const char* args);
	bool ShellExecuteOpenAndWait(const char* path, const char* args);
	void CallSystem(const char* cmd);
	I32  GetDPI();
	void CopyToClipBoard(const char* txt);

	/////////////////////////////////////////////////////////////////////////////////
	// window
	WindowRect GetClientBounds(WindowType window);
	void SetMouseCursorType(CursorType cursorType);
	void SetMouseCursorVisible(bool isVisible);

	struct WindowInitArgs {
		enum Flags {
			NO_DECORATION = 1 << 0,
			NO_TASKBAR_ICON = 1 << 1
		};
		const char* mName = "";
		bool mFullscreen = false;
		U32  mFlags = 0;
		void* mParent = nullptr;
	};
	WindowType CreateSimpleWindow(const WindowInitArgs& args);
	void DestroySimpleWindow(WindowType window);
	void SetSimpleWindowRect(WindowType window, const WindowRect& rect);
	WindowRect GetSimpleWindowRect(WindowType window);
	Connection ConnectSimpleWindowEvents(Function<void(const Event& event)> func);
	WindowPoint ToScreen(WindowType window, I32 x, I32 y);

	/////////////////////////////////////////////////////////////////////////////////
	// File 
	struct FileIterator;

	bool   FileExists(const char* path);
	bool   DirExists(const char* path);
	bool   DeleteFile(const char* path);
	bool   MoveFile(const char* from, const char* to);
	bool   FileCopy(const char* from, const char* to);
	size_t GetFileSize(const char* path);
	U64    GetLastModTime(const char* file);
	bool   CreateDir(const char* path);
	void   SetCurrentDir(const char* path);
	void   GetCurrentDir(Span<char> path);

	FileIterator* CreateFileIterator(const char* path, const char* ext = nullptr);
	void DestroyFileIterator(FileIterator* it);
	bool GetNextFile(FileIterator* it, FileInfo& info);

	void DebugOutput(const char* msg);

	/////////////////////////////////////////////////////////////////////////////////
	// library
	void* LibraryOpen(const char* path);
	void  LibraryClose(void* handle);
	void* LibrarySymbol(void* handle, const char* symbolName);
}
}