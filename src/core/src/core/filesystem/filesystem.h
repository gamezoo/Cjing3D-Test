#pragma once

#include "common\common.h"
#include "filesystem\path.h"

namespace Cjing3D 
{
namespace FileSystem
{
	bool OpenData(const char* programName, const char* assetPath, const char* assetName);
	bool IsDataOpened();
	void CloseData();
	const char* GetBasePath();

	bool CreateDirectory(const char* path);
	bool DeleteDirectory(const char* path);
	bool IsDirectoryExists(const char* path);
	std::vector<const char*> EnumerateFiles(const char* path);

	bool IsFileExists(const char* path);
	bool ReadFileBytes(const char* path, std::vector<char>& data);
	bool ReadFileBytes(const char* path, void** buffer, U32& size);
	bool SaveFile(const char* path, const char* buffer, size_t length);
	bool DeleteFile(const char* path);
}
}