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

bool Path::IsPathDir(const String& path)
{
	if (path.empty()) {
		return false;
	}

	size_t pos = path.length() - 1;
	return path[pos] == PATH_SLASH || path[pos] == PATH_BACKSLASH;
}

bool Path::IsPathFile(const String& path)
{
	String filename = GetPathBaseName(path);
	return filename.find(PATH_DOT) != String::npos;
}

String Path::FormatPath(const String& path)
{
	String ret = path;
	ret = StringUtils::ReplaceChar(ret, PATH_BACKSLASH, PATH_SEPERATOR);
	return ret;
}

String Path::CombinePath(const String& path1, const String& path2)
{
	if (path1.empty()) {
		return path2;
	}
	if (path2.empty()) {
		return path1;
	}

	auto CombineFunc = [](const String& path1, const String& path2) {
		String ret = Path::FormatPath(path1);
		if (ret.back() != PATH_SEPERATOR) {
			ret += PATH_SEPERATOR;
		}
		return Path::FormatPath(ret + path2);
	};

	bool isPath1Absoluted = IsAbsolutePath(path1);
	bool isPath2Absoluted = IsAbsolutePath(path2);

	if (isPath1Absoluted && isPath2Absoluted)
	{
		// 如果两个路径都是绝对路径则无法拼接
		return INVALID_PATH;
	}
	else if (isPath1Absoluted == isPath2Absoluted) 
	{
		// 如果两个路径都是相对路径则直接拼接
		return CombineFunc(path1, path2);
	}
	else if (isPath1Absoluted)
	{
		return CombineFunc(path1, path2);
	}
	else if (isPath2Absoluted)
	{
		return CombineFunc(path2, path1);
	}
}

String Path::GetPathParentPath(const String& path)
{
	if (path.empty() || path == INVALID_PATH) {
		return INVALID_PATH;
	}

	auto lastIndex = path.find_last_of(PATH_SEPERATOR);
	if (lastIndex == String::npos) {
		return INVALID_PATH;
	}

	return path.substr(0, lastIndex);
}

String Path::GetPathBaseName(const String& path)
{
	if (path.empty() || path == INVALID_PATH) {
		return INVALID_PATH;
	}

	String ret = path;
	ret = StringUtils::ReplaceChar(ret, PATH_BACKSLASH, PATH_SEPERATOR);

	auto lastIndex = ret.find_last_of(PATH_SEPERATOR);
	if (lastIndex != String::npos) {
		ret = ret.substr(lastIndex + 1);
	}
	return ret;
}

String Path::GetPathExtension(const String& path)
{
	if (path.empty() || path == INVALID_PATH) {
		return INVALID_PATH;
	}

	auto lastIndex = path.find_last_of(PATH_DOT);
	if (lastIndex != String::npos) {
		return path.substr(lastIndex, path.length());
	}
	return INVALID_PATH;
}

bool Path::IsAbsolutePath(const String& path)
{
	if (path.empty()) {
		return false;
	}

#ifdef CJING3D_PLATFORM_WIN32
	if (path[0] == PATH_SLASH) {
		return true;
	}
#endif

	if (path.find(":") != String::npos) {
		return true;
	}

	return false;
}

String Path::ConvertToAbsolutePath(const String& path)
{
	return String();
}

String Path::ConvertToRelativePath(const String& path)
{
	return String();
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

}