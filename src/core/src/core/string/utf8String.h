#pragma once

#include "string.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D
{
	class UTF8Iterator;

	class UTF8String final
	{
	public:
		//////////////////////////////////////////////////////////////
		using CharType				 = char;
		using StringType             = String;
		static constexpr int npos = String::npos;

		//////////////////////////////////////////////////////////////
		UTF8String() = default;
		UTF8String(const char* str);
		UTF8String(const StringType& str);
		UTF8String(Span<const char> str);
		UTF8String(StringView str);
		UTF8String(const UTF8String& rhs);
		UTF8String(UTF8String&& rhs);

		UTF8String& operator=(const UTF8String& rhs);
		UTF8String& operator=(UTF8String&& rhs);
		UTF8String& operator=(Span<const char> str);
		UTF8String& operator=(const char* str);
		UTF8String& operator=(const String& str);
		StringView operator[](size_t index) const;

		bool operator!=(const UTF8String& rhs) const;
		bool operator!=(const char* rhs) const;
		bool operator==(const UTF8String& rhs) const;
		bool operator==(const char* rhs) const;
		bool operator<(const UTF8String& rhs) const;
		bool operator>(const UTF8String& rhs) const;

		UTF8String& operator+=(const char* str) {
			append(str);
			return *this;
		}
		UTF8String& operator+=(const char c) {
			append(c);
			return *this;
		}
		UTF8String operator+(const char* str) {
			return UTF8String(*this).append(str);
		}
		UTF8String operator+(const char c) {
			return UTF8String(*this).append(c);
		}
		operator const char* () const {
			return c_str();
		}

		UTF8String& append(Span<const char> value);
		UTF8String& append(char value);
		UTF8String& append(char* value);
		UTF8String& append(const char* value);

		bool empty()const { return mLength <= 0; }
		size_t length()const { return mLength; }
		char* c_str() { return mString.c_str(); }
		const char* c_str() const { return mString.c_str(); }
		StringType& toString() { return mString; }
		const StringType& toString()const { return mString; }

		size_t     size() const;
		UTF8String substr(size_t pos, int length = -1)const;
		void       insert(size_t pos, int32_t value);
		void       insert(size_t pos, const char* value);
		void       erase(size_t pos);
		int        find(const char* str, size_t pos = 0)const;
		UTF8String back()const;
		void	   clear();
		void       replace(size_t pos, size_t len, const char* str);
		void       pop();

		DynamicArray<int32_t> GetCodePoints()const;

		UTF8Iterator iterator()const;
		UTF8Iterator begin()const;
		UTF8Iterator end()const;

	private:
		StringType mString;
		size_t mLength = 0;
	};

	class UTF8Iterator final
	{
	public:
		UTF8Iterator() = delete;
		explicit UTF8Iterator(const UTF8String& str);
		UTF8Iterator(const UTF8Iterator& rhs);
		UTF8Iterator& operator=(const UTF8Iterator& rhs);

		StringView operator*()const;

		UTF8Iterator operator+(size_t index);
		UTF8Iterator operator-(size_t index);
		UTF8Iterator& operator++();
		UTF8Iterator& operator--();
		size_t operator-(const UTF8Iterator& rhs)const;

		bool operator==(const UTF8Iterator& rhs)const;
		bool operator!=(const UTF8Iterator& rhs)const;
		bool operator<(const UTF8Iterator& rhs)const;
		bool operator>(const UTF8Iterator& rhs)const;
		bool operator<=(const UTF8Iterator& rhs)const;
		bool operator>=(const UTF8Iterator& rhs)const;

	private:
		UTF8String mString;
		size_t mIndex = 0;
	};
}