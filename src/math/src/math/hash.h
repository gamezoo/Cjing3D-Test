#pragma once

#include "maths_common.h"

#include <functional>
#include <type_traits>

namespace Cjing3D
{
	// SDBM Hash
	inline U32 SDBMHash(U32 hash, unsigned char c)
	{
		return c + (hash << 6) + (hash << 16) - hash;
	}

	inline U32 SDBHash(U32 input, const void* inData, size_t size)
	{
		const U8* data = static_cast<const U8*>(inData);
		U32 hash = input;
		while (size--) {
			hash = ((U32)*data++) + (hash << 6) + (hash << 16) - hash;
		}
		return hash;
	}

	// FNV-1a Hash
	U64 FNV1aHash(U64 input, char c);
	U64 FNV1aHash(U64 input, const void* data, size_t size);

	template<typename T>
	inline void HashCombine(U32& seed, const T& value)
	{
		std::hash<T> hasher;
		seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template<typename T>
	inline std::enable_if_t<std::is_enum<T>::value, U64> HashFunc(U64 Input, const T& Data) {
		I32 value = (I32)Data;
		return FNV1aHash(Input, &value, 4);
	}

	// Hasher
	template<typename T>
	inline std::enable_if_t<!std::is_enum<T>::value, U64> HashFunc(U64 Input, const T& Data)
	{
		static_assert(false, "This Type of Hash function not defined");
		return 0;
	}

	U64 HashFunc(U64 Input, const char* Data);
	inline U64 HashFunc(U64 Input, U32 Data) { return FNV1aHash(Input, &Data, 4); }
	inline U64 HashFunc(U64 Input, U64 Data) { return FNV1aHash(Input, &Data, sizeof(Data)); }
	inline U64 HashFunc(U64 Input, I32 Data) { return FNV1aHash(Input, &Data, 4); }
	inline U64 HashFunc(U64 Input, I64 Data) { return FNV1aHash(Input, &Data, sizeof(Data)); }

	template<typename T>
	inline U32 HashFunc(U32 Input, const T& Data)
	{
		static_assert(false, "This Type of Hash function not defined");
		return 0;
	}
	inline U32 HashFunc(U32 Input, U32 Data) { return SDBHash(Input, &Data, 4); }
	inline U32 HashFunc(U32 Input, I32 Data) { return SDBHash(Input, &Data, 4); }

	template<typename T>
	class Hasher
	{
	public:
		U32 operator()(U32 input, const T& data) const { return HashFunc(input, data); }
		U64 operator()(U64 input, const T& data) const { return HashFunc(input, data); }
	};

}