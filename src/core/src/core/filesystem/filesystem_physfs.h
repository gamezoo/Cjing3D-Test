#pragma once

#include "core\filesystem\filesystem.h"

namespace Cjing3D 
{
	class FileSystemPhysfs : public BaseFileSystem
	{
	public:
		FileSystemPhysfs(const char* basePath);
		virtual ~FileSystemPhysfs();

		virtual void SetBasePath(const char* path);
		virtual char* GetBasePath();

		virtual bool CreateDir(const char* path);
		virtual bool DeleteDir(const char* path);
		virtual bool IsDirExists(const char* path);
		virtual bool IsFileExists(const char* path);
		virtual bool ReadFile(const char* path, DynamicArray<char>& data);
		virtual bool ReadFile(const char* path, char** buffer, U32& size);
		virtual bool WriteFile(const char* path, const char* buffer, size_t length);
		virtual bool DeleteFile(const char* path);
		virtual bool OpenFile(const char* path, File& file, FileFlags flags);
		virtual U64  GetLastModTime(const char* path);
		virtual bool MoveFile(const char* from, const char* to);

		DynamicArray<String> EnumerateFiles(const char* path, int mask = EnumrateMode_ALL)override;
	private:
		MaxPathString mBasePath;
	};
}