#pragma once

#include "core\common\definitions.h"
#include "core\container\dynamicArray.h"
#include "core\filesystem\path.h"
#include "core\filesystem\file.h"

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

		virtual bool CreateDir(const char* path) = 0;
		virtual bool DeleteDir(const char* path) = 0;
		virtual bool IsDirExists(const char* path) = 0;
		virtual bool IsFileExists(const char* path) = 0;
		virtual bool ReadFile(const char* path, DynamicArray<char>& data) = 0;
		virtual bool ReadFile(const char* path, char** buffer, U32& size) = 0;
		virtual bool WriteFile(const char* path, const char* buffer, size_t length) = 0;
		virtual bool DeleteFile(const char* path) = 0;
		virtual bool OpenFile(const char* path, File& file, FileFlags flags) = 0;
		virtual U64  GetLastModTime(const char* path) = 0;
	};
}