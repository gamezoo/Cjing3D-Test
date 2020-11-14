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

	class PlatformFile
	{
	public:
		virtual ~PlatformFile() {}
		virtual bool   Read(void* buffer, size_t bytes) = 0;
		virtual bool   Write(const void* buffer, size_t bytes) = 0;
		virtual bool   Seek(size_t offset) = 0;
		virtual size_t Tell() const = 0;
		virtual size_t Size() const = 0;
		virtual FileFlags GetFlags() const = 0;
		virtual bool IsValid() const = 0;
		virtual const char* GetPath() const = 0;
	};

	class PlatformFileMem : public PlatformFile
	{
	private:
		void* mData = nullptr;
		size_t mSize = 0;
		size_t mPos = 0;
		FileFlags mFlags = FileFlags::NONE;

	public:
		PlatformFileMem(void* data, size_t size, FileFlags flags) :
			mFlags(flags),
			mData(data),
			mSize(size)
		{
		}

		~PlatformFileMem()
		{
		}

		bool Read(void* buffer, size_t bytes)override
		{
			if (FLAG_ANY(mFlags, FileFlags::READ))
			{
				const size_t copySize = std::min(mSize - mPos, bytes);
				Memory::Memcpy(buffer, (const U8*)mData + mPos, copySize);
				mPos += copySize;
				return copySize > 0;
			}
			return false;
		}

		bool Write(const void* buffer, size_t bytes)override
		{
			if (FLAG_ANY(mFlags, FileFlags::WRITE))
			{
				const size_t copySize = std::min(mSize - mPos, bytes);
				Memory::Memcpy((U8*)mData + mPos, buffer, copySize);
				mPos += copySize;
				return copySize > 0;
			}
			return false;
		}

		bool Seek(size_t offset)override
		{
			if (offset < mSize)
			{
				mPos = offset;
				return true;
			}
			return false;
		}

		size_t Tell() const override
		{
			return mPos;
		}

		size_t Size() const override {
			return mSize;
		}

		FileFlags GetFlags() const override {
			return mFlags;
		}

		bool IsValid() const override {
			return mData != nullptr;
		}

		const char* GetPath() const override {
			return "";
		}
	};

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

	class PlatformFileWin32 : public PlatformFile
	{
	private:
		size_t mSize = 0;
		FileFlags mFlags = FileFlags::NONE;
		HANDLE mHandle = INVALID_HANDLE_VALUE;
		volatile int mMappedCount = 0;
#ifdef DEBUG
		String mPath;
#endif

	public:
		PlatformFileWin32(const char* path, FileFlags flags, FilePathResolver* resolver) :
			mFlags(flags)
		{
			if (resolver != nullptr) {
				// doing something
			}

			// set flags
			DWORD desiredAccess = 0;
			DWORD shareMode = 0;
			DWORD createFlags = 0;
			DWORD fileFlags = 0;

			if (FLAG_ANY(flags, FileFlags::CREATE))
			{
				if (FileExists(path)) {
					createFlags = TRUNCATE_EXISTING;
				}
				else {
					createFlags = CREATE_ALWAYS;
				}
			}
			if (FLAG_ANY(flags, FileFlags::READ))
			{
				desiredAccess |= GENERIC_READ;
				shareMode = FILE_SHARE_READ;
			}
			if (FLAG_ANY(flags, FileFlags::WRITE))
			{
				desiredAccess |= GENERIC_WRITE;
			}

			mHandle = ::CreateFileA(path, desiredAccess, shareMode, nullptr, createFlags, fileFlags, 0);
			if (mHandle == INVALID_HANDLE_VALUE)
			{
				Debug::Error("Failed to create file:\"%s\", error:%x", path, ::GetLastError());
			}
			else
			{
				DWORD sizeH = 0;
				DWORD sizeL = ::GetFileSize(mHandle, &sizeL);
				mSize = (size_t)(sizeH )<< 32ull | sizeL;
			}

#ifdef DEBUG
			mPath = path;
#endif
		}

		~PlatformFileWin32()
		{
			if (mHandle != INVALID_HANDLE_VALUE)
			{
				::FlushFileBuffers(mHandle);
				::CloseHandle(mHandle);
			}
		}

		bool Read(void* buffer, size_t bytes)override
		{
			U8* readBuffer = static_cast<U8*>(buffer);
			DWORD readed = 0;
			BOOL success = ::ReadFile(mHandle, readBuffer, (DWORD)bytes, (LPDWORD)&readed, nullptr);
			return success && bytes == readed;
		}

		bool Write(const void* buffer, size_t bytes)override
		{
			size_t written = 0;
			const U8* writeBuffer = static_cast<const U8*>(buffer);
			BOOL success = ::WriteFile(mHandle, writeBuffer, (DWORD)bytes, (LPDWORD)&written, nullptr);
			return success && bytes == written;
		}

		bool Seek(size_t offset)override
		{
			const LONG offsetHi = offset >> 32u;
			const LONG offsetLo = offset & 0xffffffffu;
			LONG offsetHiOut = offsetHi;
			return ::SetFilePointer(mHandle, offsetLo, &offsetHiOut, FILE_BEGIN) == (DWORD)offsetLo && offsetHiOut == offsetHi;
		}

		size_t Tell() const override
		{
			LONG offsetHi = 0;
			DWORD offsetLo = ::SetFilePointer(mHandle, 0, &offsetHi, FILE_CURRENT);
			return (size_t)offsetHi << 32ull | offsetLo;
		}

		size_t Size() const override {
			return mSize;
		}

		FileFlags GetFlags() const override {
			return mFlags;
		}

		bool IsValid() const override {
			return mHandle != INVALID_HANDLE_VALUE;
		}

		const char* GetPath() const override {
#ifdef DEBUG
			return mPath.c_str();
#else
			return "";
#endif
		}
	};
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// COMMON DEFINI
////////////////////////////////////////////////////////////////////////////////////////

	File::File(const char* path, FileFlags flags, FilePathResolver* resolver)
	{
#ifdef CJING3D_PLATFORM_WIN32
		mFileImpl = CJING_NEW(PlatformFileWin32)(path, flags, resolver);
#endif
	}

	File::File(void* data, size_t size, FileFlags flags)
	{
		mFileImpl = CJING_NEW(PlatformFileMem)(data, size, flags);
	}

	File::File(File&& rhs)noexcept
	{
		std::swap(mFileImpl, rhs.mFileImpl);
	}

	File& File::operator=(File&& rhs)noexcept
	{
		std::swap(mFileImpl, rhs.mFileImpl);
		return *this;
	}

	File::~File()
	{
		CJING_SAFE_DELETE(mFileImpl);
	}

	size_t File::Read(void* buffer, size_t bytes)
	{
		Debug::CheckAssertion(FLAG_ANY(GetFlags(), FileFlags::READ));
		return mFileImpl->Read(buffer, bytes);
	}

	size_t File::Write(const void* buffer, size_t bytes)
	{
		Debug::CheckAssertion(FLAG_ANY(GetFlags(), FileFlags::WRITE));
		return mFileImpl->Write(buffer, bytes);
	}

	bool File::Seek(size_t offset)
	{
		return mFileImpl->Seek(offset);
	}

	size_t File::Tell() const
	{
		return mFileImpl->Tell();
	}

	size_t File::Size() const
	{
		return mFileImpl->Size();
	}

	FileFlags File::GetFlags() const
	{
		return mFileImpl->GetFlags();
	}

	const char* File::GetPath() const
	{
		return mFileImpl->GetPath();
	}

	bool File::IsValid() const
	{
		return mFileImpl->IsValid();
	}
}
}