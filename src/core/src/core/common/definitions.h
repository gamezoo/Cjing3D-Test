#pragma once

#include "math\maths_common.h"
#include "core\helper\function.h"

#include <assert.h>

#define NOMINMAX
#undef min
#undef max

#ifdef _MSC_VER
#pragma warning( disable :4005 4018 4522 4715 4800 4996 4267 26812)
#define COMPILER_MSVC 1
#define NOEXCEPT
#else
#define COMPILER_MSVC 0
#endif


#ifndef _ALWAYS_INLINE_
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define _ALWAYS_INLINE_ __attribute__((always_inline)) inline
#elif defined(__llvm__)
#define _ALWAYS_INLINE_ __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define _ALWAYS_INLINE_ __forceinline
#else
#define _ALWAYS_INLINE_ inline
#endif

#endif

//should always inline, except in some cases because it makes debugging harder
#ifndef _FORCE_INLINE_

#ifdef DISABLE_FORCED_INLINE
#define _FORCE_INLINE_ inline
#else
#define _FORCE_INLINE_ _ALWAYS_INLINE_
#endif

#endif

// common definitions
#ifndef _STR
#define _STR(m_x) #m_x
#define _MKSTR(m_x) _STR(m_x)
#endif

#define PLATFORM_ALIGNMENT 16
#define ALIGN_TO(n, a)((n + a)&~a)

template<class To, class From>
To CJING_DOWN_CAST(From* parent)
{
#ifdef DEBUG
	To to = dynamic_cast<To>(parent);
	return to;
#else
	return static_cast<To>(parent);
#endif
}

#ifdef CJING3D_PLATFORM_WIN32
#include <wrl.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#endif

template<typename ENUM>
constexpr inline bool FLAG_ALL(ENUM value, ENUM Flags)
{
	static_assert(sizeof(ENUM) <= sizeof(int), "Enum size too large.");
	return ((int)value & (int)Flags) == (int)Flags;
}

constexpr inline bool FLAG_ALL(int value, int Flags) { 
	return ((int)value & (int)Flags) == (int)Flags; 
}

template<typename ENUM>
constexpr inline bool FLAG_ANY(ENUM value, ENUM Flags)
{
	static_assert(sizeof(ENUM) <= sizeof(int), "Enum size too large.");
	return ((int)value & (int)Flags) != 0;
}

constexpr inline bool FLAG_ANY(int value, int Flags) {
	return ((int)value & (int)Flags) != 0; 
}