#include "packageData.h"
#include "core\helper\debug.h"
#include "core\memory\memory.h"

#include "physfs_3.0.2\physfs.h"

namespace Cjing3D {
namespace PackageData
{
	String mAssetPath = "";

	namespace FileSystemPhysfs
	{
		bool CreateDirectory(const char* name)
		{
			if (PHYSFS_isDirectory(name)) {
				return true;
			}
			return PHYSFS_mkdir(name) != 0;
		}

		bool DeleteDirectory(const char* name)
		{
			if (!PHYSFS_isDirectory(name)) {
				return true;
			}
			return PHYSFS_delete(name) != 0;
		}

		bool IsDirectoryExists(const char* name)
		{
			return PHYSFS_isDirectory(name);
		}

		bool IsFileExists(const char* name)
		{
			return PHYSFS_exists(name);
		}

		bool ReadFileBytes(const char* name, char** buffer, U32& size)
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

		bool ReadFileBytes(const char* name, std::vector<char>& data)
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

		bool WriteFileBytes(const char* name, const char* buffer, size_t length)
		{
			PHYSFS_File* file = PHYSFS_openWrite(name);
			if (file == nullptr) 
			{
				Debug::Warning(String("[fileData] The file : ") + name + " write failed.");
				return false;
			}

			if (!PHYSFS_write(file, buffer, (PHYSFS_uint32)length, 1)) 
			{
				Debug::Warning(String("[fileData] The file : ") + name + " write failed.");
				return false;
			}

			PHYSFS_close(file);
			return true;
		}

		bool DeleteFile(const char* name)
		{
			if (PHYSFS_delete(name)) {
				return false;
			}
			return true;
		}

		void EnumerateFiles(const char* path, std::vector<const char*>& fileList)
		{
			char** rc = PHYSFS_enumerateFiles(path);
			for (char** i = rc; *i != NULL; i++) {
				fileList.push_back(*i);
			}
			PHYSFS_freeList(rc);
		}
	};

	bool OpenData(const char* assetPath)
	{
		if (IsDataOpened()) {
			CloseData();
		}

		// init physfs
		PHYSFS_init(nullptr);
		PHYSFS_permitSymbolicLinks(1);

		if (!PHYSFS_mount(assetPath, nullptr, 1))
		{
			Debug::Error(String("[FileData] Failed to mount archive, path:") + assetPath + ", " + PHYSFS_getLastError());
			return false;
		}

		const String assetFullPath = assetPath;
		if (!PHYSFS_mount(assetFullPath.c_str(), nullptr, 1))
		{
			Debug::Error(String("[FileData] Failed to mount archive, path") + assetFullPath + ", " + PHYSFS_getLastError());
			return false;
		}
		const String baseDir = PHYSFS_getBaseDir();
		MaxPathString searchPath;
		Path::CombinePath(baseDir.toSpan(), assetFullPath.toSpan(), searchPath.toSpan());
		if (!PHYSFS_mount(searchPath.c_str(), nullptr, 1))
		{
			Debug::Error(String("[FileData] Failed to mount archive, path") + baseDir + "/" + assetFullPath + ", " + PHYSFS_getLastError());
			return false;
		}

		mAssetPath = assetPath != nullptr ? assetPath : "";
		return true;
	}

	bool IsDataOpened()
	{
		return PHYSFS_isInit();
	}

	void CloseData()
	{
		if (!IsDataOpened()) {
			return;
		}

		mAssetPath.clear();

		if (!PHYSFS_deinit()) {
			Debug::Error(String("[FileData] Failed to deinit fhysfs:") +  PHYSFS_getLastError());
		}
	}

	const char* GetBasePath()
	{
		return PHYSFS_getBaseDir();
	}

	bool CreateDirectory(const char* path)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::CreateDirectory(path);
		}
		return false;
	}

	bool DeleteDirectory(const char* path)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::DeleteDirectory(path);
		}
		return false;
	}

	bool IsDirectoryExists(const char* path)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::IsDirectoryExists(path);
		}
		return false;
	}

	std::vector<const char*> EnumerateFiles(const char* path)
	{
		std::vector<const char*> ret;
		if (!Path::IsAbsolutePath(path))
		{
			FileSystemPhysfs::EnumerateFiles(path, ret);
		}
		return ret;
	}

	bool IsFileExists(const char* path)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::IsFileExists(path);
		}
		return false;
	}

	bool ReadFileBytes(const char* path, std::vector<char>& data)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::ReadFileBytes(path, data);
		}
		return false;
	}

	bool ReadFileBytes(const char* path, char** buffer, U32& size)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::ReadFileBytes(path, buffer, size);
		}
		return false;
	}

	bool SaveFile(const char* path, const char* buffer, size_t length)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::WriteFileBytes(path, buffer, length);
		}
		return false;
	}

	bool DeleteFile(const char* path)
	{
		if (!Path::IsAbsolutePath(path))
		{
			return FileSystemPhysfs::DeleteFile(path);
		}
		return false;
	}
}
}