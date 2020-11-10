#pragma once

#include "core\string\string.h"

namespace Cjing3D
{
class Path
{
public:
	/// ///////////////////////////////////////////////////////////////////
	/// common function
	static const char* INVALID_PATH;
	static const char  PATH_DOT;
	static const char  PATH_SEPERATOR;
	static const char  PATH_SLASH;
	static const char  PATH_BACKSLASH;

	static bool   IsPathDir(const String& path);
	static bool   IsPathFile(const String& path);
	static String FormatPath(const String& path);
	static String CombinePath(const String& path1, const String& path2);
	static String GetPathParentPath(const String& path);
	static String GetPathBaseName(const String& path);
	static String GetPathExtension(const String& path);
	static bool   IsAbsolutePath(const String& path);
	static String ConvertToAbsolutePath(const String& path);
	static String ConvertToRelativePath(const String& path);

public:
	/// ///////////////////////////////////////////////////////////////////
	/// path definition
	Path();
	Path(const char* path);
	Path(const Path& rhs);
	Path& operator=(const Path& rhs);
	Path& operator=(const char* rhs);

	size_t Length()const { return mPath.size(); }
	const char* c_str()const { return mPath.c_str(); }
	unsigned int  GetHash()const { return mHash; }
	bool IsEmpty()const { return mPath[0] != '\0'; }

	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

private:
	static const int MAX_PATH_LENGTH = 256;
	StaticString<MAX_PATH_LENGTH> mPath;
	unsigned int mHash = 0;
};
}