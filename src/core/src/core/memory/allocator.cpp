#include "allocator.h"
#include "memTracker.h"

namespace Cjing3D
{
#ifdef CJING_MEMORY_TRACKER
	void* DefaultAllocator::Allocate(size_t size, const char* filename, int line)
	{
		void* ptr = malloc(size);
		MemoryTracker::Get().RecordAlloc(ptr, size, filename, line);
		return ptr;
	}

	void* DefaultAllocator::Reallocate(void* ptr, size_t newBytes, const char* filename, int line)
	{
		void* ret = realloc(ptr, newBytes);
		MemoryTracker::Get().RecordRealloc(ret, ptr, newBytes, filename, line);
		return ret;
	}

	void DefaultAllocator::Free(void* ptr)
	{
		MemoryTracker::Get().RecordFree(ptr);
		free(ptr);
	}

	void* DefaultAllocator::AlignAllocate(size_t size, size_t align, const char* filename, int line)
	{
		void* ptr = _aligned_malloc(size, align);
		MemoryTracker::Get().RecordAlloc(ptr, size, filename, line);
		return ptr;
	}

	void* DefaultAllocator::AlignReallocate(void* ptr, size_t newBytes, size_t align, const char* filename, int line)
	{
		void* ret = _aligned_realloc(ptr, newBytes, align);
		MemoryTracker::Get().RecordRealloc(ret, ptr, newBytes, filename, line);
		return ret;
	}

	void DefaultAllocator::AlignFree(void* ptr)
	{
		MemoryTracker::Get().RecordFree(ptr);
		return _aligned_free(ptr);
	}

#else
	void* DefaultAllocator::Allocate(size_t size)
	{
		return malloc(size);
	}

	void* DefaultAllocator::Reallocate(void* ptr, size_t newSize)
	{
		return realloc(ptr, newSize);
	}

	void DefaultAllocator::Free(void* ptr)
	{
		return free(ptr);
	}

	void* DefaultAllocator::AlignAllocate(size_t size, size_t align)
	{
		return _aligned_malloc(size, align);
	}

	void* DefaultAllocator::AlignReallocate(void* ptr, size_t newSize, size_t align)
	{
		return _aligned_realloc(ptr, newSize, align);
	}

	void DefaultAllocator::AlignFree(void* ptr)
	{
		return _aligned_free(ptr);
	}
#endif

	size_t DefaultAllocator::GetMaxAllocationSize()
	{
		return std::numeric_limits<size_t>::max();
	}
}
