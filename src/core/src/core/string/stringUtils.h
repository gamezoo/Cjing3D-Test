
#include "string.h"

namespace Cjing3D
{
namespace StringUtils
{
	bool StartsWithPrefix(const char* str, const char* prefix);
	String  ReplaceString(const String& str, const String& src, const String& dst);
	String  ReplaceChar(const String& str, char src, char dst);
	String  WStringToString(const WString& wstr);
	WString StringToWString(const String& str);
	unsigned int StringToHash(const char* str);
}
}