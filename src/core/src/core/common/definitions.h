#pragma once

#include "math\maths_common.h"
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

#define SAFE_DELETE(p) if(p!= nullptr) {delete (p); p=nullptr;}
#define SAFE_DELETE_ARRAY(p) if(p!= nullptr) {delete[](p); p=nullptr;}

// common definitions
#ifndef _STR
#define _STR(m_x) #m_x
#define _MKSTR(m_x) _STR(m_x)
#endif

#define PLATFORM_ALIGNMENT 16
#define ALIGN_TO(n, a)((n + a)&~a)

