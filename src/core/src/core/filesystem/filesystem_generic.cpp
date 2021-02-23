#include "filesystem_generic.h"
#include "core\helper\debug.h"
#include "core\memory\memory.h"
#include "core\platform\platform.h"
#include "core\filesystem\file.h"

#include "physfs_3.0.2\physfs.h"

namespace Cjing3D {

	FileSystemGeneric::FileSystemGeneric(const char* basePath)
	{
		SetBasePath(basePath);
	}

	FileSystemGeneric::~FileSystemGeneric()
	{
	}

	void FileSystemGeneric::SetBasePath(const char* path)
	{
		Path::FormatPath(path, mBasePath.toSpan());
		if (mBasePath.back() != '/' && mBasePath.back() != '\\') {
			mBasePath.append(Path::PATH_SEPERATOR);
		}
	}

	char* FileSystemGeneric::GetBasePath()
	{
		return mBasePath.c_str();
	}

	bool FileSystemGeneric::CreateDir(const char* path)
	{
		MaxPathString fullpath(mBasePath, path);
		return Platform::CreateDir(fullpath);
	}

	bool FileSystemGeneric::DeleteDir(const char* path)
	{
		MaxPathString fullpath(mBasePath, path);
		return Platform::DeleteFile(fullpath);
	}

	bool FileSystemGeneric::IsDirExists(const char* path)
	{
		MaxPathString fullpath(mBasePath, path);
		return Platform::DirExists(fullpath);
	}

	bool FileSystemGeneric::IsFileExists(const char* path)
	{
		MaxPathString fullpath(mBasePath, path);
		return Platform::FileExists(fullpath);
	}

	bool FileSystemGeneric::DeleteFile(const char* path)
	{
		MaxPathString fullpath(mBasePath, path);
		return Platform::DeleteFile(fullpath);
	}

	bool FileSystemGeneric::OpenFile(const char* path, File& file, FileFlags flags)
	{
		file = std::move(File(path, flags));
		return file.IsValid();
	}

	U64 FileSystemGeneric::GetLastModTime(const char* path)
	{
		MaxPathString fullpath(mBasePath, path);
		return Platform::GetLastModTime(fullpath.c_str());
	}

	bool FileSystemGeneric::MoveFile(const char* from, const char* to)
	{
		MaxPathString srcPath(GetBasePath(), from);
		MaxPathString toPath(GetBasePath(), to);
		return Platform::MoveFile(srcPath.c_str(), toPath.c_str());
	}

	bool FileSystemGeneric::ReadFile(const char* path, DynamicArray<char>& data)
	{
		MaxPathString fullpath(mBasePath, path);
		if (auto file = File(fullpath, FileFlags::DEFAULT_READ))
		{
			size_t size = file.Size();
			data.reserve(size);
			size_t readed = file.Read(data.data(), size);
			return readed == size;
		}
		return false;
	}

	bool FileSystemGeneric::ReadFile(const char* path, char** buffer, U32& size)
	{
		MaxPathString fullpath(mBasePath, path);
		if (auto file = File(fullpath, FileFlags::DEFAULT_READ))
		{
			size = file.Size();
			*buffer = CJING_NEW_ARR(char, size);
			size_t readed = file.Read(*buffer, size);
			return readed == size;
		}
		return false;
	}

	bool FileSystemGeneric::WriteFile(const char* path, const char* buffer, size_t length)
	{
		MaxPathString fullpath(mBasePath, path);
		if (!path || !buffer || length <= 0) {
			return false;
		}

		if (auto file = File(fullpath, FileFlags::DEFAULT_WRITE))
		{
			size_t wrote = file.Write(buffer, length);
			return wrote == length;
		}
		return false;
	}
}