
/////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM WIN32
////////////////////////////////////////////////////////////////////////////////////////
#ifdef CJING3D_PLATFORM_WIN32

#include "core\platform\platform.h"
#include "core\platform\events.h"
#include "core\memory\memory.h"
#include "core\helper\debug.h"
#include "core\string\string.h"
#include "core\string\stringUtils.h"
#include "core\signal\eventQueue.h"

#pragma warning(push)
#pragma warning(disable : 4091)
#include <ShlObj.h>
#include <commdlg.h>
#include <shellapi.h>
#pragma warning(pop)

#pragma warning(disable : 4996)

namespace Cjing3D {
namespace Platform {

	static void WCharToChar(Span<char> out, const WCHAR* in)
	{
		const WCHAR* c = in;
		char* cout = out.begin();
		const U32 size = out.length();
		while (*c && c - in < size - 1)
		{
			*cout = (char)*c;
			++cout;
			++c;
		}
		*cout = 0;
	}


	template <int N> 
	static void CharToWChar(WCHAR(&out)[N], const char* in)
	{
		const char* c = in;
		WCHAR* cout = out;
		while (*c && c - in < N - 1)
		{
			*cout = *c;
			++cout;
			++c;
		}
		*cout = 0;
	}

	template <int N>
	struct WCharString
	{
		WCharString(const char* rhs)
		{
			CharToWChar(data, rhs);
		}

		operator const WCHAR* () const
		{
			return data;
		}

		WCHAR data[N];
	};
	using WPathString = WCharString<Path::MAX_PATH_LENGTH>;

	U64 FileTimeToU64(FILETIME ft)
	{
		ULARGE_INTEGER i;
		i.LowPart = ft.dwLowDateTime;
		i.HighPart = ft.dwHighDateTime;
		return i.QuadPart;
	}

	struct PlatformImpl
	{
		struct {
			HCURSOR mArrow;
			HCURSOR mSizeALL;
			HCURSOR mSizeNS;
			HCURSOR mSizeWE;
			HCURSOR mSizeNWSE;
			HCURSOR mTextInput;
		} mCursors;

		EventQueue mEventQueue;
	};
	static PlatformImpl mImpl;

	void Initialize()
	{
		mImpl.mCursors.mArrow = LoadCursor(NULL, IDC_ARROW);
		mImpl.mCursors.mSizeALL = LoadCursor(NULL, IDC_SIZEALL);
		mImpl.mCursors.mSizeNS = LoadCursor(NULL, IDC_SIZENS);
		mImpl.mCursors.mSizeWE = LoadCursor(NULL, IDC_SIZEWE);
		mImpl.mCursors.mSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
		mImpl.mCursors.mTextInput = LoadCursor(NULL, IDC_IBEAM);
	}

	/////////////////////////////////////////////////////////////////////////////////
	// platform function
	void SetLoggerConsoleFontColor(ConsoleFontColor fontColor)
	{
		int color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		switch (fontColor)
		{
		case CONSOLE_FONT_BLUE:
			color = FOREGROUND_BLUE;
			break;
		case CONSOLE_FONT_YELLOW:
			color = FOREGROUND_RED | FOREGROUND_GREEN;
			break;
		case CONSOLE_FONT_GREEN:
			color = FOREGROUND_GREEN;
			break;
		case CONSOLE_FONT_RED:
			color = FOREGROUND_RED;
			break;
		}

		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | static_cast<WORD>(color));
	}

	void ShowMessageBox(const char* msg)
	{
		auto wStr = StringUtils::StringToWString(msg);
		MessageBoxW(NULL, LPCWSTR(wStr.c_str()), NULL, MB_OK);
	}

	void LoadFileFromOpenWindow(const char* fileFilter, std::function<void(const String&)> callback)
	{
		char szFile[256] = { '\0' };

		OPENFILENAMEA ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;

		// If you change folder in the dialog it will change the current folder for your process without OFN_NOCHANGEDIR;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn))
		{
			if (callback != nullptr) {
				callback(ofn.lpstrFile);
			}
		}
	}

	void SaveFileToOpenWindow(const char* fileFilter, std::function<void(const String&)> callback)
	{
		char szFile[256] = { '\0' };

		OPENFILENAMEA ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.Flags = OFN_OVERWRITEPROMPT;

		if (GetSaveFileNameA(&ofn))
		{
			if (callback != nullptr) {
				callback(ofn.lpstrFile);
			}
		}
	}

	void ShowBrowseForFolder(const char* title, std::function<void(const String&)> callback)
	{
		auto wStr = StringUtils::StringToWString(title);
		wchar_t szBuffer[256] = { '\0' };
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(BROWSEINFO));
		bi.hwndOwner = NULL;
		bi.pszDisplayName = szBuffer;
		bi.lpszTitle = wStr.c_str();
		bi.ulFlags = BIF_RETURNFSANCESTORS;

		LPITEMIDLIST idl = SHBrowseForFolder(&bi);
		if (NULL == idl) {
			return;
		}

		SHGetPathFromIDList(idl, szBuffer);
		callback(StringUtils::WStringToString(wStr));
	}

	bool ShellExecuteOpen(const char* path, const char* args)
	{
		const WPathString wpath(path);
		if (args != nullptr)
		{
			const WPathString wargs(args);
			const uintptr_t ret = (uintptr_t)::ShellExecute(NULL, NULL, wpath, wargs, NULL, SW_SHOW);
			return ret >= 32;
		}
		else
		{
			const uintptr_t ret = (uintptr_t)::ShellExecute(NULL, NULL, wpath, NULL, NULL, SW_SHOW);
			return ret >= 32;
		}
	}

	bool ShellExecuteOpenAndWait(const char* path, const char* args)
	{
		const WPathString wpath(path);
		SHELLEXECUTEINFO info = {};
		info.cbSize = sizeof(SHELLEXECUTEINFO);
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.hwnd = NULL;
		info.lpVerb = L"open";
		info.lpFile = wpath;

		if (args != nullptr)
		{
			const WPathString wargs(args);
			info.lpParameters = wargs;
		}
		else {
			info.lpParameters = NULL;
		}

		info.lpDirectory = NULL;
		info.nShow = SW_SHOWNORMAL;
		info.hInstApp = NULL;

		if (!::ShellExecuteEx(&info)) {
			return false;
		}

		::WaitForSingleObject(info.hProcess, INFINITE);
		return true;
	}

	void CallSystem(const char* cmd)
	{
		system(cmd);
	}

	I32 GetDPI()
	{
		const HDC hdc = GetDC(NULL);
		return GetDeviceCaps(hdc, LOGPIXELSX);
	}

	void CopyToClipBoard(const char* txt)
	{
		if (!::OpenClipboard(NULL)) {
			return;
		}

		size_t len = StringLength(txt) + 1;	
		// alloc global mem
		HGLOBAL memHandle = ::GlobalAlloc(GMEM_MOVEABLE, len * sizeof(char));
		if (!memHandle) {
			return;
		}
		char* mem = (char*)::GlobalLock(memHandle);
		CopyString(Span(mem, len), txt);
		::GlobalUnlock(memHandle);

		// set clipboard
		::EmptyClipboard();
		::SetClipboardData(CF_TEXT, memHandle);
		::CloseClipboard();
	}

	bool OpenExplorer(const char* path)
	{
		const WPathString wPath(path);
		const uintptr_t ret = (uintptr_t)::ShellExecute(NULL, L"explore", wPath, NULL, NULL, SW_SHOWNORMAL);
		return ret >= 32;
	}

	I32 GetCPUsCount()
	{
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		return sys_info.dwNumberOfProcessors > 0 ? sys_info.dwNumberOfProcessors : 1;
	}

	WindowRect GetClientBounds(WindowType window)
	{
		RECT rect;
		GetClientRect(window, &rect);
		return { (I32)rect.left, (I32)rect.top, (I32)rect.right, (I32)rect.bottom };
	}

	void SetMouseCursorType(CursorType cursorType)
	{
		switch (cursorType)
		{
		case CursorType::DEFAULT: 
			SetCursor(mImpl.mCursors.mArrow); break;
		case CursorType::SIZE_ALL: 
			SetCursor(mImpl.mCursors.mSizeALL); break;
		case CursorType::SIZE_NS:
			SetCursor(mImpl.mCursors.mSizeNS); break;
		case CursorType::SIZE_WE: 
			SetCursor(mImpl.mCursors.mSizeWE); break;
		case CursorType::SIZE_NWSE: 
			SetCursor(mImpl.mCursors.mSizeNWSE); break;
		case CursorType::TEXT_INPUT: 
			SetCursor(mImpl.mCursors.mTextInput); break;
		default: 
			break;
		}
	}

	void SetMouseCursorVisible(bool isVisible)
	{
		if (isVisible) {
			while (ShowCursor(isVisible) < 0);
		}
		else {
			while (ShowCursor(isVisible) >= 0);
		}
	}

	WindowType CreateSimpleWindow(const WindowInitArgs& args)
	{
		auto WndProc = [](HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
			switch (Msg)
			{
			case WM_MOVE:
			{
				WindowMoveEvent e;
				e.mWindow = hWnd;
				e.mMoveX = (I32)LOWORD(lParam);
				e.mMoveY = (I32)HIWORD(lParam);
				mImpl.mEventQueue.Push<WindowMoveEvent>(e);
			}
			return 0;
			case WM_SIZE:
			{
				ViewResizeEvent e;
				e.mWindow = hWnd;
				mImpl.mEventQueue.Push<ViewResizeEvent>(e);
			}
			return 0;
			case WM_CLOSE:
			{
				WindowCloseEvent e;
				e.mWindow = hWnd;
				mImpl.mEventQueue.Push<WindowCloseEvent>();
			}
			return 0;
			}
			return ::DefWindowProc(hWnd, Msg, wParam, lParam);
		};

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandle(NULL);
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = NULL;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"cjing_window";
		wcex.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

		if (!::RegisterClassEx(&wcex)) {
			Debug::CheckAssertion(false);
		}

		DWORD style = args.mFlags & WindowInitArgs::NO_DECORATION ? WS_POPUP : WS_OVERLAPPEDWINDOW;
		DWORD ext_style = args.mFlags & WindowInitArgs::NO_TASKBAR_ICON ? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW;
		WPathString wname(args.mName);
		const HWND hwnd = ::CreateWindowEx(
			ext_style,
			L"cjing_window",
			wname,
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			(HWND)args.mParent,
			NULL,
			wcex.hInstance,
			NULL);
		Debug::CheckAssertion(hwnd);

		::ShowWindow(hwnd, SW_SHOW);
		::UpdateWindow(hwnd);

		return hwnd;
	}

	void DestroySimpleWindow(WindowType window)
	{
		::DestroyWindow(window);
	}

	void SetSimpleWindowRect(WindowType window, const WindowRect& rect)
	{
		MoveWindow((HWND)window, rect.mLeft, rect.mTop, rect.mRight - rect.mLeft, rect.mBottom - rect.mTop, TRUE);
	}

	WindowRect GetSimpleWindowRect(WindowType window)
	{
		RECT rect;
		BOOL status = GetClientRect((HWND)window, &rect);
		return { rect.left, rect.top, rect.right, rect.bottom };
	}

	Connection ConnectSimpleWindowEvents(Function<void(const Event& event)> func)
	{
		return mImpl.mEventQueue.Connect(func);
	}

	WindowPoint ToScreen(WindowType window, I32 x, I32 y)
	{
		POINT p;
		p.x = x;
		p.y = y;
		::ClientToScreen((HWND)window, &p);
		return { p.x, p.y };
	}

	/////////////////////////////////////////////////////////////////////////////////
	// file function
	bool FileExists(const char* path)
	{
		const WPathString wpath(path);
		DWORD dwAttrib = GetFileAttributes(wpath);
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool DirExists(const char* path)
	{
		const WPathString wpath(path);
		DWORD dwAttrib = GetFileAttributes(wpath);
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool DeleteFile(const char* path)
	{
		const WPathString wpath(path);
		return ::DeleteFile(wpath) != FALSE;
	}

	bool MoveFile(const char* from, const char* to)
	{
		const WPathString pathFrom(from);
		const WPathString pathTo(to);
		return ::MoveFileEx(pathFrom, pathTo, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != FALSE;
	}

	bool FileCopy(const char* from, const char* to)
	{
		const WPathString pathFrom(from);
		const WPathString pathTo(to);
		BOOL retVal = ::CopyFile(pathFrom, pathTo, FALSE);
		if (retVal == FALSE)
		{
			Logger::Warning("FileCopy failed: %x", ::GetLastError());
			return false;
		}
		return true;
	}

	size_t GetFileSize(const char* path)
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		const WPathString wpath(path);
		if (!::GetFileAttributesEx(wpath, GetFileExInfoStandard, &fad)) {
			return -1;
		}

		LARGE_INTEGER size;
		size.HighPart = fad.nFileSizeHigh;
		size.LowPart  = fad.nFileSizeLow;
		return (size_t)size.QuadPart;
	}

	U64 GetLastModTime(const char* file)
	{
	/*	const WPathString wpath(file);
		FILETIME ft;
		HANDLE handle = CreateFile(wpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			return 0;
		}

		if (GetFileTime(handle, NULL, NULL, &ft) == FALSE) 
		{
			CloseHandle(handle);
			return 0;
		}
		CloseHandle(handle);

		ULARGE_INTEGER i;
		i.LowPart = ft.dwLowDateTime;
		i.HighPart = ft.dwHighDateTime;
		return (U64)i.QuadPart;*/

		struct stat attrib;
		if (stat(file, &attrib) != 0)
		{
			Logger::Warning("Faild to get last mod time \"%s\"", file);
			return 0;
		}

		return attrib.st_mtime;
	}

	bool CreateDir(const char* path)
	{
		// convert "/" to "\\"
		char temp[Path::MAX_PATH_LENGTH];
		char* out = temp;
		const char* in = path;
		while (*in && out - temp < Path::MAX_PATH_LENGTH - 1)
		{
			*out = *in == '/' ? '\\' : *in;
			++out;
			++in;
		}
		*out = '\0';

		const WPathString wpath(temp);
		return SHCreateDirectoryEx(NULL, wpath, NULL) == ERROR_SUCCESS;
	}

	void SetCurrentDir(const char* path)
	{
		WPathString tmp(path);
		::SetCurrentDirectory(tmp);
	}

	void GetCurrentDir(Span<char> path)
	{
		WCHAR tmp[Path::MAX_PATH_LENGTH];
		::GetCurrentDirectory(Path::MAX_PATH_LENGTH, tmp);
		return WCharToChar(path, tmp);
	}

	struct FileIterator
	{
		HANDLE mHandle;
		WIN32_FIND_DATA mFFD;
		bool mIsValid;
	};

	FileIterator* CreateFileIterator(const char* path, const char* ext)
	{
		char tempPath[Path::MAX_PATH_LENGTH] = { 0 };
		CopyString(tempPath, path);

		// serach specific ext files
		if (ext != nullptr)
		{
			CatString(tempPath, "/*.");
			CatString(tempPath, ext);
		}
		else
		{
			CatString(tempPath, "/*");
		}

		WCharString<Path::MAX_PATH_LENGTH> wTempPath(tempPath);
		FileIterator* it = CJING_NEW(FileIterator);
		it->mHandle = ::FindFirstFile(wTempPath, &it->mFFD);
		it->mIsValid = it->mHandle != INVALID_HANDLE_VALUE;
		return it;
	}

	void DestroyFileIterator(FileIterator* it)
	{
		::FindClose(it->mHandle);
		CJING_SAFE_DELETE(it);
	}

	bool GetNextFile(FileIterator* it, FileInfo& info)
	{
		if (!it->mIsValid) {
			return false;
		}

		WIN32_FIND_DATA& data = it->mFFD;
		WCharToChar(info.mFilename, data.cFileName);
		info.mCreatedTime  = FileTimeToU64(data.ftCreationTime);
		info.mModifiedTime = FileTimeToU64(data.ftLastWriteTime);
		info.mFileSize = ((size_t)data.nFileSizeHigh << 32LL) | (size_t)data.nFileSizeLow;
		info.mIsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		it->mIsValid = ::FindNextFile(it->mHandle, &it->mFFD) != FALSE;
		return true;
	}

	void DebugOutput(const char* msg)
	{
		WPathString tmp(msg);
		::OutputDebugString(tmp);
	}

	void* LibraryOpen(const char* path)
	{
		WCHAR tmp[Path::MAX_PATH_LENGTH];
		CharToWChar(tmp, path);
		return ::LoadLibrary(tmp);
	}

	void LibraryClose(void* handle)
	{
		::FreeLibrary((HMODULE)handle);
	}

	void* LibrarySymbol(void* handle, const char* symbolName)
	{
		return ::GetProcAddress((HMODULE)handle, symbolName);
	}

#endif
}
}