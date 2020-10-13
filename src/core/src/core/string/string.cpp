#include "string.h"

namespace Cjing3D
{
	bool StringImpl::CopyString(Span<char> dst, const char* source)
	{
		if (!source) {
			return false;
		}

		size_t length = dst.length();
		char* temp = dst.begin();
		while (*source && length > 1)
		{
			*temp = *source;
			length--;
			temp++;
			source++;
		}
		*temp = '\0';
		return *source == '\0';
	}

	bool StringImpl::CatString(Span<char> dst, const char* source)
	{
		// 1. move to the last pos of string
		size_t length = dst.length();
		char* temp = dst.begin();
		while (*temp && length > 0)
		{
			length--;
			temp++;
		}
		// 2. try to copy the source to the last pos
		return CopyString(Span(temp, length), source);
	}

	bool StringImpl::CopyNString(Span<char> dst, const char* source, size_t n)
	{
		if (!source || n <= 0) {
			return false;
		}

		size_t length = dst.length();
		char* temp = dst.begin();
		while (*source && length > 1 && n > 0)
		{
			*temp = *source;
			length--;
			temp++;
			source++;
			n--;
		}

		// if len > 0, need add '\0' in the end
		if (length > 0)
		{
			*temp = '\0';
			return *source == '\0' || n <= 0;
		}
		return false;
	}

	bool StringImpl::CatNString(Span<char> dst, const char* source, size_t n)
	{
		// 1. move to the last pos of string
		size_t length = dst.length();
		char* temp = dst.begin();
		while (*temp && length > 0)
		{
			length--;
			temp++;
		}
		// 2. try to copy the source to the last pos
		return CopyNString(Span(temp, length), source, n);
	}

	int StringImpl::CompareString(const char* lhs, const char* rhs)
	{
		return strcmp(lhs, rhs);
	}

	bool StringImpl::EqualString(const char* lhs, const char* rhs)
	{
		return strcmp(lhs, rhs) == 0;
	}

}

