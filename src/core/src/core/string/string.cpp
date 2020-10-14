#include "string.h"
#include "core\memory\memory.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
#define CJING_STRING_ALLOCATOR	CJING_MEMORY_ALLOCATOR_DEFAULT

	size_t StringImpl::StringLength(const char* str)
	{
		return strlen(str);
	}

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

	int StringImpl::FindSubstring(const char* str, const char* substr)
	{
		const char* ret = strstr(str, substr);
		return ret != nullptr ? (int)(ret - str) : -1;
	}

	int StringImpl::ReverseFindSubstring(const char* str, const char* substr)
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

	String::String(const char* str)
	{
		mSize = StringImpl::StringLength(str);
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

		if (!isSmall()) {
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
		if (str.length() < BUFFER_MINIMUN_SIZE)
		{
			if (!isSmall()) {
				gStringAllocator.Free(mBigData);
			}

			Memory::Memcpy(mSmallData, str.data(), str.length());
			mSmallData[str.length()] = '\0';
		}
		else
		{
			mBigData = (char*)gStringAllocator.Reallocate(mBigData, str.length() + 1);
			Memory::Memcpy(mBigData, str.data(), str.length());
			mBigData[str.length()] = '\0';
		}

		mSize = str.length();
		return *this;
	}

	String& String::operator=(const char* str)
	{
		*this = Span(str, StringImpl::StringLength(str));
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
				memcpy(mSmallData, mBigData, BUFFER_MINIMUN_SIZE - 1);
				mSmallData[size] = '\0';
				gStringAllocator.Free(mBigData);
			}
			else
			{
				mBigData = (char*)gStringAllocator.Reallocate(mBigData, size + 1);
				mBigData[size] = '\0';
			}
			mSize = size;
		}
	}

	char String::operator[](size_t index) const
	{
		DBG_ASSERT(index >= 0 && index < mSize);
		return isSmall() ? mSmallData[index] : mBigData[index];
	}

	bool String::operator!=(const String& rhs) const
	{
		return !StringImpl::EqualString(c_str(), rhs.c_str());
	}

	bool String::operator!=(const char* rhs) const
	{
		return !StringImpl::EqualString(c_str(), rhs);
	}

	bool String::operator==(const String& rhs) const
	{
		return StringImpl::EqualString(c_str(), rhs.c_str());
	}

	bool String::operator==(const char* rhs) const
	{
		return StringImpl::EqualString(c_str(), rhs);
	}

	bool String::operator<(const String& rhs) const
	{
		return StringImpl::CompareString(c_str(), rhs.c_str()) < 0;
	}

	bool String::operator>(const String& rhs) const
	{
		return StringImpl::CompareString(c_str(), rhs.c_str()) > 0;
	}

	String& String::cat(Span<const char> value)
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

	String& String::cat(char value)
	{
		return cat((const char*)&value);
	}

	String& String::cat(char* value)
	{
		return cat((const char*)value);
	}

	String& String::cat(const std::string& value)
	{
		return cat(value.c_str());
	}

	String String::substr(size_t pos, size_t length)
	{
		return String(c_str(), pos, length);
	}

	void String::insert(size_t pos, const char* value)
	{
		const size_t oldSize = mSize;
		size_t len = StringImpl::StringLength(value);
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

	int String::find(const char* str)
	{
		return StringImpl::FindSubstring(c_str(), str);
	}

	int String::rfind(const char* str)
	{
		return StringImpl::ReverseFindSubstring(c_str(), str);
	}

	String& String::cat(const char* value)
	{
		const size_t len = StringImpl::StringLength(value);
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

