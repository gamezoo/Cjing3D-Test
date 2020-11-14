#include "filesystem.h"
#include "core\helper\debug.h"
#include "core\memory\memory.h"
#include "core\platform\platform.h"

#include "physfs_3.0.2\physfs.h"

namespace Cjing3D {

	/// //////////////////////////////////////////////////////////////////////////////////////////////////
	/// FileSystemGeneric
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

	bool FileSystemGeneric::CreateDirectory(const char* path)
	{
		return Platform::CreateDir(path);
	}

	bool FileSystemGeneric::DeleteDirectory(const char* path)
	{
		return Platform::DeleteFile(path);
	}

	bool FileSystemGeneric::IsDirectoryExists(const char* path)
	{
		return Platform::DirExists(path);
	}

	bool FileSystemGeneric::IsFileExists(const char* path)
	{
		return Platform::FileExists(path);
	}

	bool FileSystemGeneric::DeleteFile(const char* path)
	{
		return Platform::DeleteFile(path);
	}

	bool FileSystemGeneric::ReadFileBytes(const char* path, std::vector<char>& data)
	{
		if (auto file = Platform::File(path, Platform::FileFlags::DEFAULT_READ))
		{
			size_t size = file.Size();
			data.reserve(size);
			size_t readed = file.Read(data.data(), size);
			return readed == size;
		}
		return false;
	}

	bool FileSystemGeneric::ReadFileBytes(const char* path, char** buffer, U32& size)
	{
		if (auto file = Platform::File(path, Platform::FileFlags::DEFAULT_READ))
		{
			size = file.Size();
			*buffer = CJING_NEW_ARR(char, size);
			size_t readed = file.Read(*buffer, size);
			return readed == size;
		}
		return false;
	}

	bool FileSystemGeneric::SaveFile(const char* path, const char* buffer, size_t length)
	{
		if (!path || !buffer || length <= 0) {
			return false;
		}

		if (auto file = Platform::File(path, Platform::FileFlags::DEFAULT_WRITE))
		{
			size_t wrote = file.Write(buffer, length);
			return wrote == length;
		}
		return false;
	}
  
	/// //////////////////////////////////////////////////////////////////////////////////////////////////
	/// FileSystemPhysfs
	FileSystemPhysfs::FileSystemPhysfs(const char* basePath)
	{
		PHYSFS_init(nullptr);
		PHYSFS_permitSymbolicLinks(1);
	}

	FileSystemPhysfs::~FileSystemPhysfs()
	{
		if (!PHYSFS_deinit()) {
			Debug::Error(String("[FileData] Failed to deinit fhysfs:") + PHYSFS_getLastError());
		}
	}

	void FileSystemPhysfs::SetBasePath(const char* path)
	{
		if (!PHYSFS_mount(path, nullptr, 1)) {
			Debug::Error(String("[FileData] Failed to mount archive, path:") + path + ", " + PHYSFS_getLastError());
		}
		mBasePath = path;
	}

	char* FileSystemPhysfs::GetBasePath()
	{
		return mBasePath.c_str();
	}

	bool FileSystemPhysfs::CreateDirectory(const char* name)
	{
		if (PHYSFS_isDirectory(name)) {
			return true;
		}
		return PHYSFS_mkdir(name) != 0;
	}

	bool FileSystemPhysfs::DeleteDirectory(const char* name)
	{
		if (!PHYSFS_isDirectory(name)) {
			return true;
		}
		return PHYSFS_delete(name) != 0;
	}

	bool FileSystemPhysfs::IsDirectoryExists(const char* name)
	{
		return PHYSFS_isDirectory(name);
	}

	bool FileSystemPhysfs::IsFileExists(const char* name)
	{
		return PHYSFS_exists(name);
	}

	bool FileSystemPhysfs::ReadFileBytes(const char* name, char** buffer, U32& size)
	{
		if (!PHYSFS_exists(name))
		{
			Debug::Warning(String("[fileData] The file : ") + name + " isn't exits.");
			return false;
		}

		PHYSFS_file* file = PHYSFS_openRead(name);
		if (file == nullptr)
		{
			Debug::Warning(String("[fileData] The file : ") + name + " read failed.");
			return false;
		}

		size = static_cast<size_t>(PHYSFS_fileLength(file));
		*buffer = CJING_NEW_ARR(char, size);
		PHYSFS_read(file, buffer, 1, (PHYSFS_uint32)size);
		PHYSFS_close(file);

		return true;
	}

	bool FileSystemPhysfs::ReadFileBytes(const char* name, std::vector<char>& data)
	{
		if (!PHYSFS_exists(name))
		{
			Debug::Warning(String("[fileData] The file : ") + name + " isn't exits.");
			return false;
		}

		PHYSFS_file* file = PHYSFS_openRead(name);
		if (file == nullptr)
		{
			Debug::Warning(String("[fileData] The file : ") + name + " read failed.");
			return false;
		}

		size_t size = static_cast<size_t>(PHYSFS_fileLength(file));
		data.resize(size);
		PHYSFS_read(file, data.data(), 1, (PHYSFS_uint32)size);
		PHYSFS_close(file);

		return true;
	}

	bool FileSystemPhysfs::DeleteFile(const char* name)
	{
		if (PHYSFS_delete(name)) {
			return false;
		}
		return true;
	}

	DynamicArray<const char*> FileSystemPhysfs::EnumerateFiles(const char* path)
	{
		DynamicArray<const char*> ret;
		char** rc = PHYSFS_enumerateFiles(path);
		for (char** i = rc; *i != NULL; i++) {
			ret.push(*i);
		}
		PHYSFS_freeList(rc);
		return ret;
	}


	bool SaveFile(const char* path, const char* buffer, size_t length)
	{
		PHYSFS_File* file = PHYSFS_openWrite(path);
		if (file == nullptr)
		{
			Debug::Warning(String("[fileData] The file : ") + path + " write failed.");
			return false;
		}

		if (!PHYSFS_write(file, buffer, (PHYSFS_uint32)length, 1))
		{
			Debug::Warning(String("[fileData] The file : ") + path + " write failed.");
			return false;
		}

		PHYSFS_close(file);
		return true;
	}
}