#pragma once

#include "core\common\common.h"
#include "core\filesystem\path.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D 
{

	/// //////////////////////////////////////////////////////////////////////////////////////////////////
	/// BaseFileSystem
	class BaseFileSystem
	{
	public:
		virtual ~BaseFileSystem() {}

		virtual void SetBasePath(const char* path) = 0;
		virtual char* GetBasePath() = 0;

		virtual bool CreateDirectory(const char* path) = 0;
		virtual bool DeleteDirectory(const char* path) = 0;
		virtual bool IsDirectoryExists(const char* path) = 0;
		virtual bool IsFileExists(const char* path) = 0;
		virtual bool ReadFileBytes(const char* path, std::vector<char>& data) = 0;
		virtual bool ReadFileBytes(const char* path, char** buffer, U32& size) = 0;
		virtual bool SaveFile(const char* path, const char* buffer, size_t length) = 0;
		virtual bool DeleteFile(const char* path) = 0;
	};

	/// //////////////////////////////////////////////////////////////////////////////////////////////////
	/// FileSystemGeneric
	class FileSystemGeneric : public BaseFileSystem
	{
	public:
		FileSystemGeneric(const char* basePath);
		virtual ~FileSystemGeneric();

		virtual void SetBasePath(const char* path);
		virtual char* GetBasePath();

		virtual bool CreateDirectory(const char* path);
		virtual bool DeleteDirectory(const char* path);
		virtual bool IsDirectoryExists(const char* path);
		virtual bool IsFileExists(const char* path);
		virtual bool ReadFileBytes(const char* path, std::vector<char>& data);
		virtual bool ReadFileBytes(const char* path, char** buffer, U32& size);
		virtual bool SaveFile(const char* path, const char* buffer, size_t length);
		virtual bool DeleteFile(const char* path);

	private:
		MaxPathString mBasePath;
	};

	/// //////////////////////////////////////////////////////////////////////////////////////////////////
	/// FileSystemPhysfs
	class FileSystemPhysfs : public BaseFileSystem
	{
	public:
		FileSystemPhysfs(const char* basePath);
		virtual ~FileSystemPhysfs();

		virtual void SetBasePath(const char* path);
		virtual char* GetBasePath();

		virtual bool CreateDirectory(const char* path);
		virtual bool DeleteDirectory(const char* path);
		virtual bool IsDirectoryExists(const char* path);
		virtual bool IsFileExists(const char* path);
		virtual bool ReadFileBytes(const char* path, std::vector<char>& data);
		virtual bool ReadFileBytes(const char* path, char** buffer, U32& size);
		virtual bool SaveFile(const char* path, const char* buffer, size_t length);
		virtual bool DeleteFile(const char* path);

		DynamicArray<const char*> EnumerateFiles(const char* path);

	private:
		MaxPathString mBasePath;
	};
}