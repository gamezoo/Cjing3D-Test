#include "platform.h"
#include "core\memory\memory.h"
#include "core\helper\debug.h"
#include "core\string\string.h"
#include "core\string\stringUtils.h"
#include "core\filesystem\path.h"

#ifdef CJING3D_PLATFORM_WIN32
#include <ShlObj.h>
#endif

namespace Cjing3D {
namespace Platform {

/////////////////////////////////////////////////////////////////////////////////////////
// PLATFORM WIN32
////////////////////////////////////////////////////////////////////////////////////////
#ifdef CJING3D_PLATFORM_WIN32

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
			Debug::Warning("FileCopy failed: %x", ::GetLastError());
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

	U64 GetLastModified(const char* file)
	{
		const WPathString wpath(file);
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
		return (U64)i.QuadPart;
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
#endif
}
}