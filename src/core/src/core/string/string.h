#pragma once

#include "core\container\span.h"
#include "core\common\definitions.h"

#include <stdarg.h>
#include <string_view>
#include <string>

#define USE_CUSTOM_STRING

namespace Cjing3D
{
	size_t StringLength(const char* str);
	bool CopyString(Span<char> dst, const char* source);
	bool CatChar(Span<char> dst, char source);
	bool CatString(Span<char> dst, const char* source);
	bool CopyNString(Span<char> dst, const char* source, size_t n);
	bool CatNString(Span<char> dst, const char* source, size_t n);
	int  CompareString(const char* lhs, const char* rhs);
	bool EqualString(const char* lhs, const char* rhs);
	int  FindStringChar(const char* str, const char c, int pos);
	int  ReverseFindChar(const char* str, const char c);
	int  FindSubstring(const char* str, const char* substr, int pos);
	int  ReverseFindSubstring(const char* str, const char* substr);

	template<size_t N>
	bool CopyString(char(&destination)[N], const char* source)
	{
		return CopyString(Span(destination, N), source);
	}

	template<size_t N>
	bool CatString(char(&destination)[N], const char* source)
	{
		return CatString(Span(destination, N), source);
	}

	// fixed size string 
	template<size_t N>
	class StaticString
	{
	public:
		StaticString() = default;
		StaticString(const char* str)
		{
			CopyString(mData, str);
		}

		template<typename... Args>
		StaticString(Args... args)
		{
			(append(args), ...);
		}

		template<size_t RHS_N>
		void append(const StaticString<RHS_N>& rhs) {
			CatString(mData, rhs.mData);
		}
		void append(const char* str) {
			CatString(mData, str);
		}
		void append(char* str) {
			CatString(mData, str);
		}
		void append(char c) 
		{
			char temp[2] = { c, 0 };
			CatString(mData, temp);
		}
		void append(const Span<char>& rhs)
		{
			CatNString(mData, rhs.data(), rhs.length());
		}
		void append(const std::string& rhs)
		{
			CatString(mData, rhs.data());
		}

		bool empty()const {
			return mData[0] == '\0';
		}

		char* data() {
			return mData;
		}
		const char* data()const {
			return mData;
		}
		char* c_str() { 
			return mData;
		}
		const char* c_str() const { 
			return mData;
		}

		const char back()const {
			return mData[StringLength(mData) - 1];
		}

		size_t size()const {
			return N;
		}

		size_t length()const {
			return StringLength(mData);
		}

		void operator=(const char* str) {
			CatString(mData, str);
		}

		bool operator<(const char* str) const {
			return CompareString(mData, str) < 0;
		}

		bool operator==(const char* str) const {
			return EqualString(mData, str);
		}

		bool operator!=(const char* str) const {
			return !EqualString(mData, str);
		}

		StaticString<N> operator+(const char* str) {
			return StaticString<N>(mData, str);
		}

		StaticString<N>& operator+=(const char* str) {
			append(str);
			return *this;
		}

		operator const char* () const { 
			return mData;
		}

		char& operator[](size_t index)
		{
			assert(index >= 0 && index < N);
			return mData[index];
		}

		const char& operator[](size_t index) const
		{
			assert(index >= 0 && index < N);
			return mData[index];
		}

		Span<char> toSpan() {
			return Span(mData);
		}

		Span<const char> toSpan()const {
			return Span(mData);
		}

		StaticString& Sprintf(const char* format, ...)
		{
			va_list args;
			va_start(args, format);
			vsnprintf(mData, N, format, args);
			va_end(args);
			return *this;
		}

	private:
		char mData[N] = "\0";
	};

	using String16  = StaticString<16>;
	using String32  = StaticString<32>;
	using String64  = StaticString<64>;
	using String128 = StaticString<128>;

#ifndef USE_CUSTOM_STRING
	using String = std::string;
	using StringView = std::string_view;
#else
	class String
	{
	public:
		String();
		String(const char c);
		String(const char* str);
		String(size_t size, char initChar);
		String(const String& rhs);
		String(String&& rhs);
		String(const std::string& str);
		String(Span<const char> str);
		String(const char* str, size_t pos, size_t len);
		~String();

		String& operator=(const String& rhs);
		String& operator=(String&& rhs);
		String& operator=(Span<const char> str);
		String& operator=(const char* str);
		String& operator=(const std::string& str);

		char* c_str() { return isSmall() ? mSmallData : mBigData; }
		const char* c_str() const { return isSmall() ? mSmallData : mBigData; }
		bool  empty() const { return c_str() == nullptr || c_str()[0] == '\0'; }
		char* data() { return isSmall() ? mSmallData : mBigData; }
		const char* data()const { return isSmall() ? mSmallData : mBigData; }
		size_t length()const { return mSize; }
		size_t size()const { return mSize; }
		std::string toString()const;

		char& operator[](size_t index);
		const char& operator[](size_t index) const;

		bool operator!=(const String& rhs) const;
		bool operator!=(const char* rhs) const;
		bool operator==(const String& rhs) const;
		bool operator==(const char* rhs) const;
		bool operator<(const String& rhs) const;
		bool operator>(const String& rhs) const;

		String& operator+=(const char* str) {
			append(str);
			return *this;
		}
		String& operator+=(const char c) {
			append(c);
			return *this;
		}
		String operator+(const char* str) {
			return String(*this).append(str);
		}
		String operator+(const char c) {
			return String(*this).append(c);
		}
		operator const char* () const {
			return c_str();
		}

		String& append(Span<const char> value);
		String& append(char value);
		String& append(char* value);
		String& append(const char* value);
		String& append(const std::string& value);

		void   resize(size_t size);
		String substr(size_t pos, int length = -1)const;
		void   insert(size_t pos, const char* value);
		void   erase(size_t pos);
		void   erase(size_t pos, size_t count);
		int    find(const char* str, size_t pos = 0)const;
		int    find(const char c, size_t pos = 0)const;
		int    find_last_of(const char* str)const;
		int    find_last_of(const char c)const;
		char   back()const;
		void   clear();
		void   replace(size_t pos, size_t len, const char* str);

		Span<char> toSpan();
		Span<const char> toSpan()const;

		char* begin() {
			return data();
		}
		char* end() {
			return data() + mSize;
		}

		static const int npos = -1;

	private:
		bool isSmall()const;

		// mininum buffer size
		static const size_t BUFFER_MINIMUN_SIZE = 16;
		size_t mSize = 0;
		union {
			char* mBigData = nullptr;
			char  mSmallData[BUFFER_MINIMUN_SIZE];
		};
	};

	// hash func
	U32 HashFunc(U32 Input, const String& Data);
	U64 HashFunc(U64 Input, const String& Data);

	using WString = std::wstring;
	using StringView = std::string_view;
#endif


}