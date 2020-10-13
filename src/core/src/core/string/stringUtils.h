
#include <string>

namespace Cjing3D
{
namespace StringUtils
{
	std::string  ReplaceString(const std::string& str, const std::string& src, const std::string& dst);
	std::string  ReplaceChar(const std::string& str, char src, char dst);
	std::string  WStringToString(const std::wstring& wstr);
	std::wstring StringToWString(const std::string& str);
}
}