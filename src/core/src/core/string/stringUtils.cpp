#include "stringUtils.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
namespace StringUtils
{
	namespace
	{
#ifndef CJING3D_PLATFORM_WIN32
		bool utf8CharToUcs2Char(const char* utf8Tok, wchar_t* ucs2Char, uint32_t* utf8TokLen)
		{
			//We do math, that relies on unsigned data types
			const unsigned char* utf8TokUs = reinterpret_cast<const unsigned char*>(utf8Tok);

			//Initialize return values for 'return false' cases.
			*ucs2Char = L'?';
			*utf8TokLen = 1;

			//Decode
			if (0x80 > utf8TokUs[0])
			{
				//Tokensize: 1 byte
				*ucs2Char = static_cast<const wchar_t>(utf8TokUs[0]);
			}
			else if (0xC0 == (utf8TokUs[0] & 0xE0))
			{
				//Tokensize: 2 bytes
				if (0x80 != (utf8TokUs[1] & 0xC0))
				{
					return false;
				}
				*utf8TokLen = 2;
				*ucs2Char = static_cast<const wchar_t>(
					(utf8TokUs[0] & 0x1F) << 6
					| (utf8TokUs[1] & 0x3F)
					);
			}
			else if (0xE0 == (utf8TokUs[0] & 0xF0))
			{
				//Tokensize: 3 bytes
				if ((0x80 != (utf8TokUs[1] & 0xC0))
					|| (0x80 != (utf8TokUs[2] & 0xC0))
					)
				{
					return false;
				}
				*utf8TokLen = 3;
				*ucs2Char = static_cast<const wchar_t>(
					(utf8TokUs[0] & 0x0F) << 12
					| (utf8TokUs[1] & 0x3F) << 6
					| (utf8TokUs[2] & 0x3F)
					);
			}
			else if (0xF0 == (utf8TokUs[0] & 0xF8))
			{
				//Tokensize: 4 bytes
				*utf8TokLen = 4;
				return false;                        //Character exceeds the UCS-2 range (UCS-4 would be necessary)
			}
			else if ((0xF8 == utf8TokUs[0] & 0xFC))
			{
				//Tokensize: 5 bytes
				*utf8TokLen = 5;
				return false;                        //Character exceeds the UCS-2 range (UCS-4 would be necessary)
			}
			else if (0xFC == (utf8TokUs[0] & 0xFE))
			{
				//Tokensize: 6 bytes
				*utf8TokLen = 6;
				return false;                        //Character exceeds the UCS-2 range (UCS-4 would be necessary)
			}
			else
			{
				return false;
			}

			return true;
		}

		void ucs2CharToUtf8Char(const wchar_t ucs2Char, char* utf8Tok)
		{
			//We do math, that relies on unsigned data types
			uint32_t ucs2CharValue = static_cast<uint32_t>(ucs2Char);   //The standard doesn't specify the signed/unsignedness of wchar_t
			unsigned char* utf8TokUs = reinterpret_cast<unsigned char*>(utf8Tok);

			//Decode
			if (0x80 > ucs2CharValue)
			{
				//Tokensize: 1 byte
				utf8TokUs[0] = static_cast<unsigned char>(ucs2CharValue);
				utf8TokUs[1] = '\0';
			}
			else if (0x800 > ucs2CharValue)
			{
				//Tokensize: 2 bytes
				utf8TokUs[2] = '\0';
				utf8TokUs[1] = static_cast<unsigned char>(0x80 | (ucs2CharValue & 0x3F));
				ucs2CharValue = (ucs2CharValue >> 6);
				utf8TokUs[0] = static_cast<unsigned char>(0xC0 | ucs2CharValue);
			}
			else
			{
				//Tokensize: 3 bytes
				utf8TokUs[3] = '\0';
				utf8TokUs[2] = static_cast<unsigned char>(0x80 | (ucs2CharValue & 0x3F));
				ucs2CharValue = (ucs2CharValue >> 6);
				utf8TokUs[1] = static_cast<unsigned char>(0x80 | (ucs2CharValue & 0x3F));
				ucs2CharValue = (ucs2CharValue >> 6);
				utf8TokUs[0] = static_cast<unsigned char>(0xE0 | ucs2CharValue);
			}
		}

		std::wstring utf8ToUcs2(const String& utf8Str)
		{
			std::wstring ucs2Result;
			wchar_t ucs2CharToStrBuf[] = { 0, 0 };
			const char* cursor = utf8Str.c_str();
			const char* const end = utf8Str.c_str() + utf8Str.length();

			while (end > cursor)
			{
				uint32_t utf8TokLen = 1;
				utf8CharToUcs2Char(cursor, &ucs2CharToStrBuf[0], &utf8TokLen);
				ucs2Result.append(ucs2CharToStrBuf);
				cursor += utf8TokLen;
			}

			return ucs2Result;
		}

		String ucs2ToUtf8(const std::wstring& ucs2Str)
		{
			String utf8Result;
			char utf8Sequence[] = { 0, 0, 0, 0, 0 };
			const wchar_t* cursor = ucs2Str.c_str();
			const wchar_t* const end = ucs2Str.c_str() + ucs2Str.length();

			while (end > cursor)
			{
				const wchar_t ucs2Char = *cursor;
				ucs2CharToUtf8Char(ucs2Char, utf8Sequence);
				utf8Result.append(utf8Sequence);
				cursor++;
			}

			return utf8Result;
		}
#endif
	}

	String ReplaceString(const String& str, const String& src, const String& dst)
	{
		if (src == dst) {
			return str;
		}

		String out = str;
		size_t pos = str.find(src, 0);
		while (pos != String::npos)
		{
			out.replace(pos, src.size(), dst);
			pos = out.find(src, pos + dst.size());
		}

		return out;
	}

	String ReplaceChar(const String& str, char src, char dst)
	{
		String out = str;
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

	String WStringToString(const WString& wstr)
	{
#ifdef CJING3D_PLATFORM_WIN32
		if (wstr.empty()) return String();
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		String strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
#else
		return ucs2ToUtf8(wstr);
#endif
	}

	WString StringToWString(const String& str)
	{
#ifdef CJING3D_PLATFORM_WIN32
		if (str.empty()) return std::wstring();
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
#else
		return utf8ToUcs2(str);
#endif
	}

}
}