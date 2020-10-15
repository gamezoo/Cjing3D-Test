#include "string.h"
#include "core\memory\memory.h"
#include "core\helper\debug.h"

#include <string>

namespace Cjing3D
{
#define CJING_STRING_ALLOCATOR	CJING_MEMORY_ALLOCATOR_DEFAULT

	size_t StringLength(const char* str)
	{
		return strlen(str);
	}

	bool CopyString(Span<char> dst, const char* source)
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

	bool CatString(Span<char> dst, const char* source)
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

	bool CopyNString(Span<char> dst, const char* source, size_t n)
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

	bool CatNString(Span<char> dst, const char* source, size_t n)
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

	int CompareString(const char* lhs, const char* rhs)
	{
		return strcmp(lhs, rhs);
	}

	bool EqualString(const char* lhs, const char* rhs)
	{
		return strcmp(lhs, rhs) == 0;
	}

	int FindStringChar(const char* str, const char c, int pos)
	{
		const char* ret = strchr(str + pos, c);
		return ret != nullptr ? (int)(ret - str) : -1;
	}

	int ReverseFindChar(const char* str, const char c)
	{
		const char* ret = strrchr(str, c);
		return ret != nullptr ? (int)(ret - str) : -1;
	}

	int FindSubstring(const char* str, const char* substr, int pos)
	{
		const char* ret = strstr(str + pos, substr);
		return ret != nullptr ? (int)(ret - str) : -1;
	}

	int ReverseFindSubstring(const char* str, const char* substr)
	{
		return std::string(str).rfind(substr);
	}

#if (CJING_MEMORY_ALLOCATOR == CJING_MEMORY_ALLOCATOR_DEFAULT)
	DefaultAllocator gStringAllocator;
#endif

	String::String()
	{
		mSize = 0;
		mSmallData[0] = '\0';
	}

	String::String(const char c)
	{
		mSize = 1;
		mSmallData[0] = c;
		mSmallData[1] = '\0';
	}

	String::String(const char* str)
	{
		mSize = StringLength(str);
		if (mSize < BUFFER_MINIMUN_SIZE)
		{
			Memory::Memcpy(mSmallData, str, mSize + 1);
		}
		else
		{
			mBigData = (char*)gStringAllocator.Allocate(mSize + 1);
			Memory::Memcpy(mBigData, str, mSize + 1);
		}
	}

	String::String(size_t size, char initChar)
	{
		resize(size);
		Memory::Memset(data(), initChar, size);
	}

	String::String(const String& rhs)
	{
		mSize = 0;
		*this = Span(rhs.c_str(), rhs.length());
	}

	String::String(String&& rhs)
	{
		if (isSmall())
		{
			Memory::Memcpy(mSmallData, rhs.mSmallData, sizeof(mSmallData));
			rhs.mSmallData[0] = '\0';
		}
		else
		{
			mBigData = rhs.mBigData;
			rhs.mBigData = nullptr;
		}
		mSize = rhs.mSize;
		rhs.mSize = 0;
	}

	String::String(const std::string& str)
	{
		mSize = 0;
		*this = Span(str.c_str(), str.length());
	}

	String::String(Span<const char> str)
	{
		mSize = 0;
		*this = str;
	}

	String::String(const char* str, size_t pos, size_t len)
	{
		mSize = 0;
		*this = Span(str + pos, len);
	}

	String::~String()
	{
		if (!isSmall()) {
			gStringAllocator.Free(mBigData);
		}
	}

	String& String::operator=(const String& rhs)
	{
		if (&rhs == this) {
			return *this;
		}
		*this = Span(rhs.c_str(), rhs.length());
		return *this;
	}

	String& String::operator=(String&& rhs)
	{
		if (&rhs == this) {
			return *this;
		}

		if (!isSmall()) 
		{
			gStringAllocator.Free(mBigData);
		}

		if (rhs.isSmall())
		{
			Memory::Memcpy(mSmallData, rhs.mSmallData, sizeof(mSmallData));
			rhs.mSmallData[0] = '\0';
		}
		else
		{
			mBigData = rhs.mBigData;
			rhs.mBigData = nullptr;
		}

		mSize = rhs.mSize;
		rhs.mSize = 0;
		return *this;
	}

	String& String::operator=(Span<const char> str)
	{
		if (!isSmall()) 
		{
			gStringAllocator.Free(mBigData);
		}

		if (str.length() < BUFFER_MINIMUN_SIZE)
		{
			Memory::Memcpy(mSmallData, str.data(), str.length());
			mSmallData[str.length()] = '\0';
		}
		else
		{
			mBigData = (char*)gStringAllocator.Allocate(str.length() + 1);
			Memory::Memcpy(mBigData, str.data(), str.length());
			mBigData[str.length()] = '\0';
		}

		mSize = str.length();
		return *this;
	}

	String& String::operator=(const char* str)
	{
		*this = Span(str, StringLength(str));
		return *this;
	}

	String& String::operator=(const std::string& str)
	{
		*this = Span(str.c_str(), str.length());
		return *this;
	}

	void String::resize(size_t size)
	{
		if (size <= 0) {
			return;
		}

		// need to keep original data
		if (isSmall())
		{
			if (size < BUFFER_MINIMUN_SIZE)
			{
				mSmallData[size] = '\0';
			}
			else
			{
				char* tmp = (char*)gStringAllocator.Allocate(size + 1);
				Memory::Memcpy(tmp, mSmallData, mSize + 1);
				mBigData = tmp;
			}
			mSize = size;
		}
		else
		{
			if (size < BUFFER_MINIMUN_SIZE)
			{
				char* tmp = mBigData;
				memcpy(mSmallData, tmp, BUFFER_MINIMUN_SIZE - 1);
				mSmallData[size] = '\0';
				gStringAllocator.Free(tmp);
			}
			else
			{
				mBigData = (char*)gStringAllocator.Reallocate(mBigData, size + 1);
				mBigData[size] = '\0';
			}
			mSize = size;
		}
	}

	char& String::operator[](size_t index)
	{
		DBG_ASSERT(index >= 0 && index < mSize);
		return isSmall() ? mSmallData[index] : mBigData[index];
	}

	const char& String::operator[](size_t index) const
	{
		DBG_ASSERT(index >= 0 && index < mSize);
		return isSmall() ? mSmallData[index] : mBigData[index];
	}

	bool String::operator!=(const String& rhs) const
	{
		return !EqualString(c_str(), rhs.c_str());
	}

	bool String::operator!=(const char* rhs) const
	{
		return !EqualString(c_str(), rhs);
	}

	bool String::operator==(const String& rhs) const
	{
		return EqualString(c_str(), rhs.c_str());
	}

	bool String::operator==(const char* rhs) const
	{
		return EqualString(c_str(), rhs);
	}

	bool String::operator<(const String& rhs) const
	{
		return CompareString(c_str(), rhs.c_str()) < 0;
	}

	bool String::operator>(const String& rhs) const
	{
		return CompareString(c_str(), rhs.c_str()) > 0;
	}

	String& String::append(Span<const char> value)
	{
		if (value.length() == 0) {
			return *this;
		}

		size_t oldSize = mSize;
		resize(oldSize + value.length());
		Memory::Memcpy(data() + oldSize, value.data(), value.length());
		data()[oldSize + value.length()] = '\0';
		return *this;
	}

	String& String::append(char value)
	{
		return append(Span(&value, 1));
	}

	String& String::append(char* value)
	{
		return append((const char*)value);
	}

	String& String::append(const std::string& value)
	{
		return append(value.c_str());
	}

	String String::substr(size_t pos, int length)const
	{
		length = length < 0 ? mSize - pos : std::min((int)mSize - (int)pos, length);
		return String(c_str(), pos, std::max(0, length));
	}

	void String::insert(size_t pos, const char* value)
	{
		const size_t oldSize = mSize;
		size_t len = StringLength(value);
		resize(oldSize + len);

		char* temp = data();
		Memory::Memmove(temp + pos + len, temp + pos, oldSize - pos);
		Memory::Memcpy(temp + pos, value, len);
	}

	void String::earse(size_t pos)
	{
		if (pos >= mSize) {
			return;
		}

		char* temp = data();
		Memory::Memmove(temp + pos, temp + pos + 1, mSize - pos - 1);
		mSize--;
		temp[mSize] = '\0';
	}

	int String::find(const char* str, size_t pos)const
	{
		pos = std::min(mSize - 1, pos);
		return FindSubstring(c_str(), str, pos);
	}

	int String::find(const char c, size_t pos)const
	{
		pos = std::min(mSize - 1, pos);
		return FindStringChar(c_str(), c, pos);
	}

	int String::find_last_of(const char* str)const
	{
		return ReverseFindSubstring(c_str(), str);
	}

	int String::find_last_of(const char c)const
	{
		return ReverseFindChar(c_str(), c);
	}

	char String::back() const
	{
		return mSize > 0 ? c_str()[mSize - 1] : '\0';
	}

	void String::clear()
	{
		if (!isSmall()) 
		{
			gStringAllocator.Free(mBigData);
		}

		mSize = 0;
		mSmallData[0] = '\0';
	}

	void String::replace(size_t pos, size_t len, const char* str)
	{
		if (pos >= mSize ||
			len > mSize - pos) {
			return;
		}

		const size_t srcLen = StringLength(str);
		if (srcLen == len)
		{
			// if same length, just copy new str to dest
			Memory::Memcpy(data() + pos, str, len);
		}
		else if (pos + len < mSize)
		{
			const size_t oldSize = mSize;
			if (srcLen > len)
			{
				resize(oldSize + (srcLen - len));
				char* temp = data();
				Memory::Memmove(temp + pos + srcLen, temp + pos + len, oldSize - pos - len);
				Memory::Memcpy(temp + pos, str, srcLen);
			}
			else
			{
				char* temp = data();
				Memory::Memmove(temp + pos + srcLen, temp + pos + len, oldSize - pos - len);
				resize(oldSize + (srcLen - len));
				Memory::Memcpy(data() + pos, str, srcLen);
			}
		}
		else
		{
			resize(mSize + (srcLen - len));
			Memory::Memcpy(data() + pos, str, srcLen);
		}
	}

	std::string String::toString() const
	{
		return std::string(c_str());
	}

	String& String::append(const char* value)
	{
		const size_t len = StringLength(value);
		if (len == 0) {
			return *this;
		}

		size_t oldSize = mSize;
		resize(oldSize + len);
		Memory::Memcpy(data() + oldSize, value, len + 1);
		return *this;
	}

	bool String::isSmall() const
	{
		return mSize < BUFFER_MINIMUN_SIZE;
	}
}

