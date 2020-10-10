#pragma once

#include <string>

namespace Cjing3D
{
class Path
{
public:
	static const char* INVALID_PATH;
	static const char  PATH_DOT;
	static const char  PATH_SEPERATOR;
	static const char  PATH_SLASH;
	static const char  PATH_BACKSLASH;

	static bool IsPathDir(const std::string& path);
	static bool IsPathFile(const std::string& path);
	static std::string FormatPath(const std::string& path);
	static std::string CombinePath(const std::string& path1, const std::string& path2);
	static std::string GetPathParentPath(const std::string& path);
	static std::string GetPathBaseName(const std::string& path);
	static std::string GetPathExtension(const std::string& path);
	static bool IsAbsolutePath(const std::string& path);
	static std::string ConvertToAbsolutePath(const std::string& path);
	static std::string ConvertToRelativePath(const std::string& path);
};
}