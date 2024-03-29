#include "filesystem_physfs.h"
#include "core\helper\debug.h"
#include "core\memory\memory.h"
#include "core\platform\platform.h"
#include "core\filesystem\file.h"

#include "physfs_3.0.2\physfs.h"

namespace Cjing3D {

	class PhsysFile : public FileImpl
	{
	private:
		PHYSFS_file* mFile = nullptr;
		size_t mSize = 0;
		FileFlags mFlags = FileFlags::NONE;

	public:
		PhsysFile(PHYSFS_file* file,  FileFlags flags) :
			mFlags(flags),
			mFile(file),
			mSize(PHYSFS_fileLength(file))
		{
		}

		~PhsysFile()
		{
			if (mFile != nullptr) {
				PHYSFS_close(mFile);
			}
		}

		bool Read(void* buffer, size_t bytes)override
		{
			if (FLAG_ANY(mFlags, FileFlags::READ))
			{
				const size_t copySize = std::min(mSize - PHYSFS_tell(mFile), bytes);
				size_t readed = PHYSFS_read(mFile, buffer, 1, (PHYSFS_uint32)copySize);
				return copySize == readed;
			}
			return false;
		}

		bool Write(const void* buffer, size_t bytes)override
		{
			if (FLAG_ANY(mFlags, FileFlags::WRITE)) {
				size_t wrote = PHYSFS_write(mFile, buffer, 1, (PHYSFS_uint32)bytes);
				return wrote == bytes;
			}
			return false;
		}

		bool Seek(size_t offset)override
		{
			if (offset < mSize)
			{
				PHYSFS_seek(mFile, offset);
				return true;
			}
			return false;
		}

		size_t Tell() const override
		{
			return PHYSFS_tell(mFile);
		}

		size_t Size() const override {
			return mSize;
		}

		FileFlags GetFlags() const override {
			return mFlags;
		}

		bool IsValid() const override {
			return mFile != nullptr;
		}

		void Close()override
		{
			if (mFile != nullptr) {
				PHYSFS_close(mFile);
			}
		}

		const char* GetPath() const override {
			return "";
		}
	};

	FileSystemPhysfs::FileSystemPhysfs(const char* basePath)
	{
		PHYSFS_init(nullptr);
		PHYSFS_permitSymbolicLinks(1);
		SetBasePath(basePath);
	}

	FileSystemPhysfs::~FileSystemPhysfs()
	{
		if (!PHYSFS_deinit()) {
			Logger::Error(String("[FileData] Failed to deinit fhysfs:") + PHYSFS_getLastError());
		}
	}

	void FileSystemPhysfs::SetBasePath(const char* path)
	{
		if (!PHYSFS_mount(path, nullptr, 1)) {
			Logger::Error(String("[FileData] Failed to mount archive, path:") + path + ", " + PHYSFS_getLastError());
		}
	
		if (!PHYSFS_setWriteDir(path)) {
			Logger::Error(String("[FileData] Failed to set write path:") + PHYSFS_getLastError());
		}

		mBasePath = path;
	}

	char* FileSystemPhysfs::GetBasePath()
	{
		return mBasePath.c_str();
	}

	bool FileSystemPhysfs::CreateDir(const char* name)
	{
		if (PHYSFS_isDirectory(name)) {
			return true;
		}
		return PHYSFS_mkdir(name) != 0;
	}

	bool FileSystemPhysfs::DeleteDir(const char* name)
	{
		if (!PHYSFS_isDirectory(name)) {
			return true;
		}
		return PHYSFS_delete(name) != 0;
	}

	bool FileSystemPhysfs::IsDirExists(const char* name)
	{
		return PHYSFS_isDirectory(name);
	}

	bool FileSystemPhysfs::IsFileExists(const char* name)
	{
		return PHYSFS_exists(name);
	}

	bool FileSystemPhysfs::ReadFile(const char* name, char** buffer, U32& size)
	{
		if (!PHYSFS_exists(name))
		{
			Logger::Warning(String("[fileData] The file : ") + name + " isn't exits.");
			return false;
		}

		PHYSFS_file* file = PHYSFS_openRead(name);
		if (file == nullptr)
		{
			Logger::Warning(String("[fileData] The file : ") + name + " read failed.");
			return false;
		}

		I32 length = static_cast<size_t>(PHYSFS_fileLength(file));
		*buffer = (char*)CJING_MALLOC(sizeof(char) * length);
		auto readSize = PHYSFS_read(file, *buffer, 1, (PHYSFS_uint32)length);
		if (readSize != length)
		{
			Logger::Warning("[fileData] The file \"%s\" read failed,%s", name, PHYSFS_getLastError());
			return false;
		}
		PHYSFS_close(file);
		size = length;
		return true;
	}

	bool FileSystemPhysfs::ReadFile(const char* name, DynamicArray<char>& data)
	{
		if (!PHYSFS_exists(name))
		{
			Logger::Warning(String("[fileData] The file : ") + name + " isn't exits.");
			return false;
		}

		PHYSFS_file* file = PHYSFS_openRead(name);
		if (file == nullptr)
		{
			Logger::Warning(String("[fileData] The file : ") + name + " read failed.");
			return false;
		}

		size_t size = static_cast<size_t>(PHYSFS_fileLength(file));
		data.resize(size);
		
		auto readed = PHYSFS_read(file, data.data(), 1, (PHYSFS_uint32)size);
		if (readed != size)
		{
			Logger::Warning("[fileData] The file \"%s\" read failed,%s", name, PHYSFS_getLastError());
			return false;
		}

		PHYSFS_close(file);

		return true;
	}

	bool FileSystemPhysfs::DeleteFile(const char* name)
	{
		if (!PHYSFS_delete(name)) {
			return false;
		}
		return true;
	}

	bool FileSystemPhysfs::OpenFile(const char* path, File& file, FileFlags flags)
	{
		if (FLAG_ANY(flags, FileFlags::READ))
		{
			if (!PHYSFS_exists(path))
			{
				Logger::Warning("[fileData] The file \"%s\" is not exists.", path);
				return false;
			}

			PHYSFS_file* physfsFile = PHYSFS_openRead(path);
			if (physfsFile == nullptr)
			{
				Logger::Warning("[fileData] The file \"%s\" open failed.", path);
				return false;
			}

			file = std::move(File(CJING_NEW(PhsysFile)(physfsFile, flags)));
			return true;
		}
		else if (FLAG_ANY(flags, FileFlags::WRITE))
		{
			PHYSFS_file* physfsFile = PHYSFS_openWrite(path);
			if (physfsFile == nullptr)
			{
				Logger::Warning("[fileData] The file \"%s\" open failed.", path);
				return false;
			}

			file = std::move(File(CJING_NEW(PhsysFile)(physfsFile, flags)));
			return true;
		}
	}

	U64 FileSystemPhysfs::GetLastModTime(const char* path)
	{
		return PHYSFS_getLastModTime(path);
	}

	bool FileSystemPhysfs::MoveFile(const char* from, const char* to)
	{
		MaxPathString srcPath(GetBasePath(), Path::PATH_SEPERATOR, from);
		MaxPathString toPath(GetBasePath(), Path::PATH_SEPERATOR, to);
		return Platform::MoveFile(srcPath.c_str(), toPath.c_str());
	}

	DynamicArray<String> FileSystemPhysfs::EnumerateFiles(const char* path, int mask)
	{
		DynamicArray<String> ret;
		char** rc = EqualString(path, ".") ? PHYSFS_enumerateFiles("") : PHYSFS_enumerateFiles(path);
		for (char** i = rc; *i != NULL; i++) 
		{
			if (mask & EnumrateMode_DIRECTORY && PHYSFS_isDirectory(*i))
			{
				ret.push(*i);
				continue;
			}
			if (mask & EnumrateMode_FILE && !PHYSFS_isDirectory(*i))
			{
				ret.push(*i);
				continue;
			}
		}
		PHYSFS_freeList(rc);
		return ret;
	}

	bool FileSystemPhysfs::WriteFile(const char* path, const char* buffer, size_t length)
	{
		PHYSFS_File* file = PHYSFS_openWrite(path);
		if (file == nullptr)
		{
			Logger::Warning(String("[fileData] The file : ") + path + " write failed.");
			return false;
		}

		if (!PHYSFS_write(file, buffer, (PHYSFS_uint32)length, 1))
		{
			Logger::Warning(String("[fileData] The file : ") + path + " write failed.");
			return false;
		}

		PHYSFS_close(file);
		return true;
	}
}