#pragma once

#include "maths_common.h"
#include "vec_facade.h"
#include "mat_facade.h"

namespace Cjing3D
{
	template <class T>
	inline T Clamp(T a, T lower, T upper)
	{
		if (a < lower) {
			return lower;
		}
		else if (a > upper) {
			return upper;
		}
		else {
			return a;
		}
	}

	template <class T>
	inline T Saturate(T v)
	{

		v = std::max(v, (T)0);
		v = std::min(v, (T)1);
		return v;
	}

	template <class T>
	inline T Saturated(T& v)
	{
		v = std::max(v, (T)0);
		v = std::min(v, (T)1);
		return v;
	}

	template <class S, class T>
	inline S Lerp(const S& value0, const S& value1, T f)
	{
		return (1 - f) * value0 + f * value1;
	}


	// SDBM Hash
	inline unsigned int SDBMHash(unsigned int hash, unsigned char c)
	{
		return c + (hash << 6) + (hash << 16) - hash;
	}

	template<typename T>
	inline void HashCombine(U32& seed, const T& value)
	{
		std::hash<T> hasher;
		seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}