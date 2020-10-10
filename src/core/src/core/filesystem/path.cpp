#include "path.h"
#include "string\stringUtils.h"
#include "platform\platform.h"
#include "filesystem\filesystem.h"

namespace Cjing3D
{
const char* Path::INVALID_PATH = "N/A";

#ifdef CJING3D_PLATFORM_WIN32
const char Path::PATH_DOT       = '.';
const char Path::PATH_SEPERATOR = '/';
const char Path::PATH_SLASH     = '/';
const char Path::PATH_BACKSLASH = '\\';
#endif

bool Path::IsPathDir(const std::string& path)
{
	if (path.empty()) {
		return false;
	}

	size_t pos = path.length() - 1;
	return path[pos] == PATH_SLASH || path[pos] == PATH_BACKSLASH;
}

bool Path::IsPathFile(const std::string& path)
{
	std::string filename = GetPathBaseName(path);
	return filename.find(PATH_DOT) != std::string::npos;
}

std::string Path::FormatPath(const std::string& path)
{
	std::string ret = path;
	ret = StringUtils::ReplaceChar(ret, PATH_BACKSLASH, PATH_SEPERATOR);
	return ret;
}

std::string Path::CombinePath(const std::string& path1, const std::string& path2)
{
	if (path1.empty()) {
		return path2;
	}
	if (path2.empty()) {
		return path1;
	}

	auto CombineFunc = [](const std::string& path1, const std::string& path2) {
		std::string ret = Path::FormatPath(path1);
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

std::string Path::GetPathParentPath(const std::string& path)
{
	if (path.empty() || path == INVALID_PATH) {
		return INVALID_PATH;
	}

	auto lastIndex = path.find_last_of(PATH_SEPERATOR);
	if (lastIndex == std::string::npos) {
		return INVALID_PATH;
	}

	return path.substr(0, lastIndex);
}

std::string Path::GetPathBaseName(const std::string& path)
{
	if (path.empty() || path == INVALID_PATH) {
		return INVALID_PATH;
	}

	std::string ret = path;
	ret = StringUtils::ReplaceChar(ret, PATH_BACKSLASH, PATH_SEPERATOR);

	auto lastIndex = ret.find_last_of(PATH_SEPERATOR);
	if (lastIndex != std::string::npos) {
		ret = ret.substr(lastIndex + 1);
	}
	return ret;
}

std::string Path::GetPathExtension(const std::string& path)
{
	if (path.empty() || path == INVALID_PATH) {
		return INVALID_PATH;
	}

	auto lastIndex = path.find_last_of(PATH_DOT);
	if (lastIndex != std::string::npos) {
		return path.substr(lastIndex, path.length());
	}
	return INVALID_PATH;
}

bool Path::IsAbsolutePath(const std::string& path)
{
	if (path.empty()) {
		return false;
	}

#ifdef CJING3D_PLATFORM_WIN32
	if (path[0] == PATH_SLASH) {
		return true;
	}
#endif

	if (path.find(":") != std::string::npos) {
		return true;
	}

	return false;
}

std::string Path::ConvertToAbsolutePath(const std::string& path)
{
	return std::string();
}

std::string Path::ConvertToRelativePath(const std::string& path)
{
	return std::string();
}

}