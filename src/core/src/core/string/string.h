#pragma once

#include "core\container\span.h"

#include <string>

#define USE_CUSTOM_STRING

namespace Cjing3D
{
	namespace StringImpl
	{
		bool CopyString(Span<char> dst, const char* source);
		bool CatString(Span<char> dst, const char* source);
		bool CopyNString(Span<char> dst, const char* source, size_t n);
		bool CatNString(Span<char> dst, const char* source, size_t n);
		int  CompareString(const char* lhs, const char* rhs);
		bool EqualString(const char* lhs, const char* rhs);

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

	};
#endif
}