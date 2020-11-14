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

	enum class FileFlags
	{
		NONE = 0,
		READ = 1 << 0,
		WRITE = 1 << 1,
		CREATE = 1 << 2,
		MMAP = 1 << 3,

		DEFAULT_READ = READ | MMAP,
		DEFAULT_WRITE = WRITE | CREATE,
	};

	class FilePathResolver;
	class File
	{
	public:
		File() = default;
		File(const char* path, FileFlags flags, FilePathResolver* resolver = nullptr);
		File(void* data, size_t size, FileFlags flags = FileFlags::READ);
		File(File&& rhs)noexcept;
		File& operator=(File&& rhs)noexcept;
		~File();

		size_t Read(void* buffer, size_t bytes);
		size_t Write(const void* buffer, size_t bytes);
		bool Seek(size_t offset);
		size_t Tell() const;
		size_t Size() const;
		FileFlags GetFlags() const;
		const char* GetPath() const;
		bool IsValid() const;

		explicit operator bool() const { return IsValid(); }

	private:
		File(const File&) = delete;

		class PlatformFile* mFileImpl = nullptr;
	};
}
}