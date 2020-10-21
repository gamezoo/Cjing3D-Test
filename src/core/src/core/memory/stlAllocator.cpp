#include "stlAllocator.h"
#include "memory.h"

namespace Cjing3D
{
	DefaultAllocator mAllocator;

	void* STLAllocPolicy::Allocate(size_t size)
	{
		return CJING_ALLOCATOR_MALLOC(mAllocator, size) ;
	}

	void STLAllocPolicy::Free(void* ptr)
	{
		return CJING_ALLOCATOR_FREE(mAllocator, ptr);
	}

	size_t STLAllocPolicy::GetMaxAllocationSize()
	{
		return mAllocator.GetMaxAllocationSize();
	}
}