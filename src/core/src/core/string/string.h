#pragma once

#include "core\container\span.h"

#include <string>

#define USE_CUSTOM_STRING

namespace Cjing3D
{
	namespace StringImpl
	{
		size_t StringLength(const char* str);
		bool CopyString(Span<char> dst, const char* source);
		bool CatString(Span<char> dst, const char* source);
		bool CopyNString(Span<char> dst, const char* source, size_t n);
		bool CatNString(Span<char> dst, const char* source, size_t n);
		int  CompareString(const char* lhs, const char* rhs);
		bool EqualString(const char* lhs, const char* rhs);
		int  FindSubstring(const char* str, const char* substr);
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
	}

	// fixed size string 
	template<size_t N>
	class StaticString
	{
	public:
		StaticString() = default;
		StaticString(const char* str)
		{
			StringImpl::CopyString(mData, str);
		}

		template<typename... Args>
		StaticString(Args... args)
		{
			(append(args), ...);
		}

		template<size_t RHS_N>
		void append(const StaticString<RHS_N>& rhs) {
			StringImpl::CatString(mData, rhs.mData);
		}
		void append(const char* str) {
			StringImpl::CatString(mData, str);
		}
		void append(char* str) {
			StringImpl::CatString(mData, str);
		}
		void append(char c) 
		{
			char temp[2] = { c, 0 };
			StringImpl::CatString(mData, temp);
		}
		void append(const Span<char>& rhs)
		{
			StringImpl::CatNString(mData, rhs.data(), rhs.length());
		}
		void append(const std::string& rhs)
		{
			StringImpl::CatString(mData, rhs.data());
		}

		bool empty()const {
			return mData[0] == '\0';
		}

		void operator=(const char* str) {
			StringImpl::CatString(mData, str);
		}

		bool operator<(const char* str) const {
			return StringImpl::CompareString(mData, str) < 0;
		}

		bool operator==(const char* str) const {
			return StringImpl::EqualString(mData, str);
		}

		bool operator!=(const char* str) const {
			return !StringImpl::EqualString(mData, str);
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

	private:
		char mData[N] = "\0";
	};

	using String32  = StaticString<32>;
	using String64  = StaticString<64>;
	using String128 = StaticString<128>;

#ifndef USE_CUSTOM_STRING
	using String = std::string;
#else
	class String
	{
	public:
		String();
		String(const char* str);
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

		void resize(size_t size);
		char* c_str() { return isSmall() ? mSmallData : mBigData; }
		const char* c_str() const { return isSmall() ? mSmallData : mBigData; }
		bool  empty() const { return c_str() == nullptr || c_str()[0] == '\0'; }
		char* data() { return isSmall() ? mSmallData : mBigData; }
		size_t length()const { return mSize; }

		char operator[](size_t index) const;
		bool operator!=(const String& rhs) const;
		bool operator!=(const char* rhs) const;
		bool operator==(const String& rhs) const;
		bool operator==(const char* rhs) const;
		bool operator<(const String& rhs) const;
		bool operator>(const String& rhs) const;

		String& operator+=(const char* str) {
			cat(str);
			return *this;
		}

		String operator+(const char* str) {
			return cat(str);
		}

		operator const char* () const {
			return c_str();
		}

		String& cat(Span<const char> value);
		String& cat(char value);
		String& cat(char* value);
		String& cat(const char* value);
		String& cat(const std::string& value);

		String substr(size_t pos, size_t length);
		void insert(size_t pos, const char* value);
		void earse(size_t pos);
		int find(const char* str);
		int rfind(const char* str);

		static const int npos = -1;

		char* begin() {
			return data();
		}
		char* end() {
			return data() + mSize;
		}

	private:
		bool isSmall()const;

		// mininum buffer size
		static const size_t BUFFER_MINIMUN_SIZE = 16;
		size_t mSize = 0;
		union {
			char* mBigData;
			char  mSmallData[BUFFER_MINIMUN_SIZE];
		};
	};
#endif
}