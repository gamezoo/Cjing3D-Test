#pragma once

#include "core\string\string.h"

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

	static bool IsPathDir(const String& path);
	static bool IsPathFile(const String& path);
	static String FormatPath(const String& path);
	static String CombinePath(const String& path1, const String& path2);
	static String GetPathParentPath(const String& path);
	static String GetPathBaseName(const String& path);
	static String GetPathExtension(const String& path);
	static bool IsAbsolutePath(const String& path);
	static String ConvertToAbsolutePath(const String& path);
	static String ConvertToRelativePath(const String& path);
};
}