#pragma once

#include "core\string\string.h"

namespace Cjing3D
{
class Path
{
public:
	static const int MAX_PATH_LENGTH = 256;

	/// ///////////////////////////////////////////////////////////////////
	/// common function
	static const char* INVALID_PATH;
	static const char  PATH_DOT;
	static const char  PATH_SEPERATOR;
	static const char  PATH_SLASH;
	static const char  PATH_BACKSLASH;

	static bool IsPathDir(const char* path);
	static bool IsPathFile(const char* path);
	static void	FormatPath(const char* path, Span<char> outPath);
	static void CombinePath(Span<const char> path1, Span<const char> path2, Span<char> out);
	static void GetPathParentPath(const char* path, Span<char> out);
	static void GetPathBaseName(const char* path, Span<char> basename);
	static void GetPathExtension(Span<const char> path, Span<char> out);
	static bool IsAbsolutePath(const char* path);
	static void ConvertToAbsolutePath(Span<char> path, Span<char> out);
	static void ConvertToRelativePath(Span<char> path, Span<char> out);

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
	bool IsEmpty()const { return mPath[0] == '\0'; }
	bool SplitPath(char* outPath, size_t pathLen, char* outFile = nullptr, size_t fileLen = 0, char* outExt = nullptr, size_t extLen = 0)const;
	void AppendPath(const Path& path);
	void AppendPath(const char* path);
	void AppendString(const char* str);
	void Normalize();
	bool IsAbsolutePath()const;
	void Clear();

	Span<char> toSpan() {
		return mPath.toSpan();
	}

	Span<const char> toSpan()const {
		return mPath.toSpan();
	}

	bool operator==(const Path& rhs) const;
	bool operator!=(const Path& rhs) const;

private:
	StaticString<MAX_PATH_LENGTH> mPath;
	unsigned int mHash = 0;
};
using MaxPathString = StaticString<Path::MAX_PATH_LENGTH>;

class FilePathResolver
{
public:
	FilePathResolver() {}
	virtual ~FilePathResolver() {}

	virtual bool ResolvePath(const char* inPath, char* outPath, size_t maxOutPath) = 0;
	virtual bool OriginalPath(const char* inPath, char* outPath, size_t maxOutPath) = 0;
};

}