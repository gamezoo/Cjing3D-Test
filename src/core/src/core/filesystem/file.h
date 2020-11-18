#pragma once

#include "core\common\definitions.h"

namespace Cjing3D
{
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

	class FileImpl
	{
	public:
		virtual ~FileImpl() {}
		virtual bool   Read(void* buffer, size_t bytes) = 0;
		virtual bool   Write(const void* buffer, size_t bytes) = 0;
		virtual bool   Seek(size_t offset) = 0;
		virtual size_t Tell() const = 0;
		virtual size_t Size() const = 0;
		virtual FileFlags GetFlags() const = 0;
		virtual bool IsValid() const = 0;
		virtual const char* GetPath() const = 0;
	};

	class FilePathResolver;
	class File
	{
	public:
		File() = default;
		File(const char* path, FileFlags flags, FilePathResolver* resolver = nullptr);
		File(void* data, size_t size, FileFlags flags = FileFlags::READ);
		File(File&& rhs)noexcept;
		File(FileImpl* fileImpl);
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

		FileImpl* mFileImpl = nullptr;
	};
}