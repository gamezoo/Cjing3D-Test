#include "stringUtils.h"

namespace Cjing3D
{
namespace StringUtils
{
	std::string ReplaceString(const std::string& str, const std::string& src, const std::string& dst)
	{
		if (src == dst) {
			return str;
		}

		std::string out = str;
		size_t pos = str.find(src, 0);
		while (pos != std::string::npos)
		{
			out.replace(pos, src.size(), dst);
			pos = out.find(src, pos + dst.size());
		}

		return out;
	}

	std::string ReplaceChar(const std::string& str, char src, char dst)
	{
		std::string out = str;
		if (out.empty())
		{
			return out;
		}
		char* buf = &(*out.begin());
		while (*buf)
		{
			if (*buf == src)
				*buf = dst;
			buf++;
		}

		return out;
	}

}
}