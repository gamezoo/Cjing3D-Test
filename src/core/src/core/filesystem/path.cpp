#include "path.h"
#include "core\string\stringUtils.h"
#include "core\platform\platform.h"
#include "core\filesystem\filesystem.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{

/// ///////////////////////////////////////////////////////////////////
/// common function
const char* Path::INVALID_PATH = "N/A";

#ifdef CJING3D_PLATFORM_WIN32
const char Path::PATH_DOT       = '.';
const char Path::PATH_SEPERATOR = '/';
const char Path::PATH_SLASH     = '/';
const char Path::PATH_BACKSLASH = '\\';
#endif

bool Path::IsPathDir(const char* path)
{
	if (path == nullptr) {
		return false;
	}

	size_t length = StringLength(path);
	if (length <= 0) {
		return false;
	}

	return path[length - 1] == PATH_SLASH || path[length - 1] == PATH_BACKSLASH;
}

bool Path::IsPathFile(const char* path)
{
	MaxPathString basename;
	GetPathBaseName(path, Span(basename.data(), basename.size()));
	return FindStringChar(basename.c_str(), PATH_DOT, 0) != -1;
}

void Path::FormatPath(const char* path, Span<char> outPath)
{
	if (path == nullptr) {
		return;
	}

	char* out = outPath.begin();
	U32 maxLength = outPath.length();
	U32 i = 0;
	bool isPrevSlash = false;

	if (path[0] == '.' && (path[1] == '\\' || path[1] == '/')) {
		path += 2;
	}
#ifdef CJING3D_PLATFORM_WIN32
	if (path[0] == '\\' || path[0] == '/') {
		++path;
	}
#endif
	while (*path != '\0' && i < maxLength)
	{
		bool isCurrentSlash = *path == '\\' || *path == '/';
		if (isCurrentSlash && isPrevSlash)
		{
			++path;
			continue;
		}
		*out = *path == '\\' ? '/' : *path;

		path++;
		out++;
		i++;
		isPrevSlash = isCurrentSlash;
	}
	(i < maxLength ? *out : *(out - 1)) = '\0';
}

void Path::CombinePath(Span<const char> path1, Span<const char> path2, Span<char> out)
{
	if (path1.empty() || path2.empty()) {
		return;
	}

	FormatPath(path1.data(), out);
	if (StringLength(path1.data()) > 0) {
		CatChar(out, PATH_SEPERATOR);
	}

	CatString(out, path2.data());
}

void Path::GetPathParentPath(const char* path, Span<char> out)
{
	CopyString(out, path);
	for (int i = StringLength(path) - 1; i >= 0; --i)
	{
		if (out[i] == '\\' || out[i] == '/')
		{
			++i;
			out[i] = '\0';
			return;
		}
	}
	out[0] = '\0';
}

void Path::GetPathBaseName(const char* path, Span<char> basename)
{
	basename[0] = '\0';
	for (int i = StringLength(path) - 1; i >= 0; --i)
	{
		if (path[i] == '\\' || path[i] == '/' || i == 0)
		{
			if (path[i] == '\\' || path[i] == '/') {
				++i;
			}

			U32 j = 0;
			basename[j] = path[i];
			while (j < basename.length() - 1 && path[i + j] && path[i + j] != '.')
			{
				++j;
				basename[j] = path[j + i];
			}
			basename[j] = '\0';
			return;
		}
	}
}

void Path::GetPathExtension(Span<const char> path, Span<char> out)
{
	if (path.empty()) {
		return;
	}

	for (int i = path.length() - 1; i >= 0; --i)
	{
		if (path[i] == '.')
		{
			++i;
			Span<const char> tmp = { path.begin() + i, path.end() };
			CopyString(out, tmp.data());
			return;
		}
	}
	out[0] = '\0';
}

bool Path::IsAbsolutePath(const char* path)
{
	if (path == nullptr) {
		return false;
	}
	
#ifdef CJING3D_PLATFORM_WIN32
	if (path[0] == PATH_SLASH) {
		return true;
	}
#endif
	if (FindStringChar(path, ':', 0) != -1) {
		return true;
	}
	return false;
}

void Path::ConvertToAbsolutePath(Span<char> path, Span<char> out)
{
}

void Path::ConvertToRelativePath(Span<char> path, Span<char> out)
{
}

/// ///////////////////////////////////////////////////////////////////
/// path definition
Path::Path() :
	mHash(0),
	mPath()
{
}

Path::Path(const char* path):
	mPath(path),
	mHash(StringUtils::StringToHash(path))
{
}

Path::Path(const Path& rhs) :
	mPath(rhs.mPath),
	mHash(rhs.mHash)
{
}

Path& Path::operator=(const Path& rhs)
{
	mPath = rhs.mPath;
	mHash = rhs.mHash;
	return *this;
}

Path& Path::operator=(const char* rhs)
{
	mPath = rhs;
	mHash = StringUtils::StringToHash(rhs);
	return *this;
}

bool Path::operator==(const Path& rhs) const
{
	return mHash == rhs.mHash;
}

bool Path::operator!=(const Path& rhs) const
{
	return mHash != mHash;
}

bool Path::SplitPath(char* outPath, size_t pathLen, char* outFile, size_t fileLen, char* outExt, size_t extLen)const
{
	if (IsEmpty()) {
		return false;
	}

	StaticString<MAX_PATH_LENGTH> temp = mPath;
	size_t length = (size_t)temp.length();
	size_t spos = length;

	// extension
	if (spos > 0)
	{
		for (size_t i = spos - 1; i >= 0; i--)
		{
			char c = temp[i];
			if (c == '\\' || c == '/')
			{
				outExt[0] = '\0';
				spos = length;
				break;
			}
			// Found extension.
			else if (c == '.')
			{
				if (outExt && extLen > 0) {
					strcpy_s(outExt, extLen, &temp[i + 1]);
				}
				temp[i] = '\0';
				spos = i;
				break;
			}
		}
	}

	// filename
	if (spos > 0)
	{
		for (size_t i = spos - 1; i >= 0; --i)
		{
			char c = temp[i];
			if (i == 0)
			{
				if (outFile && fileLen > 0) {
					strcpy_s(outFile, fileLen, &temp[i]);
				}
				spos = i;
				break;
			}
			else if (c == '\\' || c == '/')
			{
				if (outFile && fileLen > 0) {
					strcpy_s(outFile, fileLen, &temp[i + 1]);
				}
				temp[i] = '\0';
				spos = i;
				break;
			}
		}
	}

	// path
	if (outPath && pathLen > 0 && spos > 0)
	{
		strcpy_s(outPath, pathLen, temp.data());
	}

	return true;
}

void Path::AppendPath(const Path& path)
{
	if (path.IsEmpty()) {
		return;
	}

	if (IsEmpty()) {
		*this = path;
		return;
	}

	bool isPath1Absoluted = IsAbsolutePath();
	bool isPath2Absoluted = path.IsAbsolutePath();
	if (isPath1Absoluted && isPath2Absoluted) {
		return;
	}
	
	Normalize();

	if (mPath.back() != PATH_SEPERATOR) {
		mPath.append(PATH_SEPERATOR);
	}
	mPath.append(path.mPath);
}

void Path::Normalize()
{
	if (IsEmpty()) {
		return;
	}

	StaticString<MAX_PATH_LENGTH> tempPath = mPath;
	FormatPath(tempPath.c_str(), Span(mPath.data(), mPath.size()));
}

bool Path::IsAbsolutePath() const
{
	return FindStringChar(mPath.c_str(), ':', 0) != -1;
}

}