
#include "string.h"

namespace Cjing3D
{
namespace StringUtils
{
	String  ReplaceString(const String& str, const String& src, const String& dst);
	String  ReplaceChar(const String& str, char src, char dst);
	String  WStringToString(const WString& wstr);
	WString StringToWString(const String& str);
}
}