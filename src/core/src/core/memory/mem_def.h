#pragma once

#include "core\common\definitions.h"

///////////////////////////////////////////////////////////////////////
// allocator definitions
#define CJING_MEMORY_ALLOCATOR_DEFAULT 1

///////////////////////////////////////////////////////////////////////
// memory tracker
#ifdef DEBUG
#define CJING_MEMORY_TRACKER
#endif

///////////////////////////////////////////////////////////////////////
// common memory allocator
#define CJING_MEMORY_ALLOCATOR	CJING_MEMORY_ALLOCATOR_DEFAULT

// container allocator
#define CJING_CONTAINER_ALLOCATOR  CJING_MEMORY_ALLOCATOR_DEFAULT

///////////////////////////////////////////////////////////////////////
// use stl smart pointer
#define USE_STL_SMART_POINTER