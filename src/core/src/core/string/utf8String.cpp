#include "utf8String.h"
#include "utf8.h"
#include "core\helper\debug.h"

#include <algorithm>

namespace Cjing3D
{
#ifdef __cplusplus
#define utf8_null NULL
#else
#define utf8_null 0
#endif
	namespace {

		bool CheckUTF8StringValid(const char* str)
		{
			return utf8valid(str) == utf8_null;
		}

		size_t GetUTF8PosAt(const char* str, size_t index)
		{
			size_t pos = 0;
			const char* v = str;
			for (size_t i = 0; i < index && *v; i++)
			{
				size_t size = utf8codepointcalcsize(v);
				pos += size;
				v += size;
			}
			return pos;
		}

		Span<const char> SubstrUTF8String(const char* str, size_t startPos, size_t endPos)
		{
			if (startPos > endPos) {
				return Span<const char>();
			}

			const char* sp = str + GetUTF8PosAt(str, startPos);
			size_t len = GetUTF8PosAt(sp, endPos - startPos);
			return Span(sp, len);
		}

		size_t GetUTF8PosAndSize(const char* str, size_t index, size_t count, size_t& totalSize)
		{
			totalSize = 0;
			size_t pos = 0;
			const char* v = str;
			for (size_t i = 0; i < (index + count) && *v; i++)
			{
				size_t size = utf8codepointcalcsize(v);
				pos += i < index ? size : 0;
				totalSize += size;
				v += size;
			}
			totalSize -= pos;
			return pos;
		}

		int FindUTF8Substring(const char* str, const char* substr, int pos)
		{
			const char* v = str;
			if (pos > 0) {
				v += GetUTF8PosAt(str, pos);
			}

			void* ret = utf8str(str, substr);
			if (ret == utf8_null) {
				return UTF8String::npos;
			}

			pos = 0;
			for (; *v && v != ret; v += utf8codepointcalcsize(v)) {
				pos++;
			}
			return pos;
		}
	}

	UTF8String::UTF8String(const char* str)
	{
		if (!CheckUTF8StringValid(str))
		{
			Logger::Warning("Invalid utf8 string:%s", str);
			return;
		}
		mString = str;
		mLength = utf8len(str);
	}

	UTF8String::UTF8String(const StringType& str) :
		UTF8String(str.data())
	{
	}

	UTF8String::UTF8String(Span<const char> str)
	{
		mLength = 0;
		*this = str;
	}

	UTF8String::UTF8String(StringView str)
	{
		mLength = 0;
		*this = Span(str.data(), str.size());
	}

	UTF8String::UTF8String(const UTF8String& rhs)
	{
		mString = rhs.mString;
		mLength = rhs.mLength;
	}

	UTF8String::UTF8String(UTF8String&& rhs)
	{
		mString = std::move(rhs.mString);
		mLength = rhs.mLength;
		rhs.mLength = 0;
	}

	UTF8String& UTF8String::operator=(const UTF8String& rhs)
	{
		if (&rhs == this) {
			return *this;
		}

		mString = rhs.mString;
		mLength = rhs.mLength;

		return *this;
	}

	UTF8String& UTF8String::operator=(UTF8String&& rhs)
	{
		if (&rhs == this) {
			return *this;
		}

		mString = std::move(rhs.mString);
		mLength = rhs.mLength;
		rhs.mLength = 0;
	}

	UTF8String& UTF8String::operator=(Span<const char> str)
	{
		if (!CheckUTF8StringValid(str.data()))
		{
			Logger::Warning("Invalid utf8 string:%s", str);
			mLength = 0;
			return *this;
		}

		mString = str;
		mLength = utf8len(str.data());
	}

	UTF8String& UTF8String::operator=(const char* str)
	{
		*this = Span(str, StringLength(str));
		return *this;
	}

	UTF8String& UTF8String::operator=(const String& str)
	{
		*this = Span(str.data(), str.size());
		return *this;
	}

	StringView UTF8String::operator[](size_t index) const
	{
		size_t pos = GetUTF8PosAt(mString, index);
		const UTF8String::CharType* data = mString.data() + pos;
		return { data, utf8codepointcalcsize(data) };
	}

	bool UTF8String::operator!=(const UTF8String& rhs) const
	{
		return utf8cmp(c_str(), rhs.c_str()) != 0;
	}

	bool UTF8String::operator!=(const char* rhs) const
	{
		return utf8cmp(c_str(), rhs) != 0;
	}

	bool UTF8String::operator==(const UTF8String& rhs) const
	{
		return utf8cmp(c_str(), rhs.c_str()) == 0;
	}

	bool UTF8String::operator==(const char* rhs) const
	{
		return utf8cmp(c_str(), rhs) == 0;
	}

	bool UTF8String::operator<(const UTF8String& rhs) const
	{
		return utf8cmp(c_str(), rhs.c_str()) < 0;
	}

	bool UTF8String::operator>(const UTF8String& rhs) const
	{
		return utf8cmp(c_str(), rhs.c_str()) > 0;
	}

	UTF8String& UTF8String::append(Span<const char> value)
	{
		if (!CheckUTF8StringValid(value.data()))
		{
			Logger::Warning("Invalid utf8 string:%s", value);
			return *this;
		}
		mString.append(value);
		mLength = utf8len(mString.data());
	}

	UTF8String& UTF8String::append(char value)
	{
		return append(Span(&value, 1));
	}

	UTF8String& UTF8String::append(char* value)
	{
		return append((const char*)value);
	}

	UTF8String& UTF8String::append(const char* value)
	{
		if (!CheckUTF8StringValid(value))
		{
			Logger::Warning("Invalid utf8 string:%s", value);
			return *this;
		}
		mString.append(value);
		mLength = utf8len(mString.data());
	}

	size_t UTF8String::size() const
	{
		return utf8size(mString.data());
	}

	UTF8String UTF8String::substr(size_t pos, int length) const
	{
		if (pos >= mLength) {
			return UTF8String();
		}

		size_t substrCount = (length == npos || (pos + length > mLength)) ? mLength - pos : length;
		return UTF8String(SubstrUTF8String(mString, pos, pos + substrCount));
	}

	void UTF8String::insert(size_t pos, int32_t value)
	{
		size_t size = utf8codepointsize(value);
		if (size <= 0) {
			return;
		}
		mString.resize(mString.size() + size);

		pos = std::min(pos, mLength);
		size_t utf8Pos = GetUTF8PosAt(mString, pos);
		char* strData = mString.data() + utf8Pos;
		memcpy(strData + size, strData, mString.size() - size - utf8Pos);

		auto result = utf8catcodepoint(strData, value, size);
		if (!result) {
			return;
		}

		mLength++;
	}

	void UTF8String::insert(size_t pos, const char* value)
	{
		if (!CheckUTF8StringValid(value))
		{
			Logger::Warning("Invalid utf8 string:%s", value);
			return;
		}

		size_t utf8Pos = GetUTF8PosAt(mString, pos);
		mString.insert(utf8Pos, value);
		mLength += utf8len(value);
	}

	void UTF8String::erase(size_t pos)
	{
		if (pos >= mLength) {
			return;
		}

		size_t size = 0;
		size_t utf8Pos = GetUTF8PosAndSize(mString, pos, 1, size);
		mString.erase(utf8Pos, size);
		mLength--;
	}

	int UTF8String::find(const char* str, size_t pos) const
	{
		pos = std::min(mLength - 1, pos);
		return FindUTF8Substring(c_str(), str, pos);
	}

	UTF8String UTF8String::back() const
	{
		if (mLength <= 0) {
			return UTF8String();
		}

		return substr(mLength - 1, 1);
	}

	void UTF8String::clear()
	{
		mString.clear();
		mLength = 0;
	}

	void UTF8String::replace(size_t pos, size_t len, const char* str)
	{
		if (pos >= mLength ||
			len > mLength - pos) {
			return;
		}

		if (!CheckUTF8StringValid(str))
		{
			Logger::Warning("Invalid utf8 string:%s", str);
			return;
		}

		size_t size = 0;
		size_t utf8Pos = GetUTF8PosAndSize(mString, pos, len, size);
		mString.replace(utf8Pos, size, str);
		mLength = utf8len(mString);
	}

	void UTF8String::pop()
	{
		size_t pos = GetUTF8PosAt(mString, mLength - 1);
		mString.erase(pos);
		mLength--;
	}

	DynamicArray<int32_t> UTF8String::GetCodePoints() const
	{
		DynamicArray<int32_t> ret;
		utf8_int32_t codepoint;
		const char* data = mString.data();
		for (void* v = utf8codepoint(data, &codepoint); codepoint; v = utf8codepoint(v, &codepoint)) {
			ret.push(codepoint);
		}
		return ret;
	}

	UTF8Iterator UTF8String::iterator() const
	{
		return UTF8Iterator(*this);
	}

	UTF8Iterator UTF8String::begin() const
	{
		return iterator();
	}

	UTF8Iterator UTF8String::end() const
	{
		return iterator() + mLength;
	}


	//////////////////////////////////////////////////////////////////////////////
	// iterator

	UTF8Iterator::UTF8Iterator(const UTF8String& str) :
		mString(str), mIndex(0)
	{
	}

	UTF8Iterator::UTF8Iterator(const UTF8Iterator& rhs) :
		mString(rhs.mString), mIndex(rhs.mIndex)
	{
	}

	UTF8Iterator& UTF8Iterator::operator=(const UTF8Iterator& rhs)
	{
		mString = rhs.mString;
		mIndex = rhs.mIndex;
		return *this;
	}

	StringView UTF8Iterator::operator*() const
	{
		return mString[mIndex];
	}

	UTF8Iterator UTF8Iterator::operator+(size_t index)
	{
		UTF8Iterator it(*this);
		const size_t len = mString.length();
		it.mIndex = mIndex + index < len ? mIndex + index : len;
		return it;
	}

	UTF8Iterator UTF8Iterator::operator-(size_t index)
	{
		UTF8Iterator it(*this);
		const size_t len = mString.length();
		it.mIndex = mIndex - index >= 0 ? mIndex - index : 0;
		return it;
	}

	UTF8Iterator& UTF8Iterator::operator++()
	{
		if (mIndex < mString.length())
			mIndex++;

		return *this;
	}

	UTF8Iterator& UTF8Iterator::operator--()
	{
		if (mIndex > 0)
			mIndex--;

		return *this;
	}

	size_t UTF8Iterator::operator-(const UTF8Iterator& rhs) const
	{
		return mIndex - rhs.mIndex;
	}

	bool UTF8Iterator::operator==(const UTF8Iterator& rhs) const
	{
		return (mString == rhs.mString) && (mIndex == rhs.mIndex);
	}

	bool UTF8Iterator::operator!=(const UTF8Iterator& rhs) const
	{
		return (mString != rhs.mString) || (mIndex != rhs.mIndex);
	}

	bool UTF8Iterator::operator<(const UTF8Iterator& rhs) const
	{
		return (mString == rhs.mString) && (mIndex < rhs.mIndex);
	}

	bool UTF8Iterator::operator>(const UTF8Iterator& rhs) const
	{
		return (mString == rhs.mString) && (mIndex > rhs.mIndex);
	}

	bool UTF8Iterator::operator<=(const UTF8Iterator& rhs) const
	{
		return (mString == rhs.mString) && (mIndex <= rhs.mIndex);
	}

	bool UTF8Iterator::operator>=(const UTF8Iterator& rhs) const
	{
		return (mString == rhs.mString) && (mIndex >= rhs.mIndex);
	}
}