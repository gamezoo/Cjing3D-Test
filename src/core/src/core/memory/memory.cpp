#include "memory.h"

namespace Cjing3D
{
	MemoryAllocator allocator;

#ifdef CJING_MEMORY_TRACKER
	void* Memory::Alloc(size_t size, const char* filename, int line)
	{
		void* ptr = allocator.Allocate(size);
		MemoryTracker::Get().RecordAlloc(ptr, size, filename, line);
		return ptr;
	}

	void* Memory::Realloc(void* ptr, size_t newBytes, const char* filename, int line)
	{
		ptr = allocator.Reallocate(ptr, newBytes);
		MemoryTracker::Get().RecordRealloc(ptr, newBytes, filename, line);
		return ptr;
	}

	void Memory::Free(void* ptr)
	{
		MemoryTracker::Get().RecordFree(ptr);
		allocator.Free(ptr);
	}

	void* Memory::AlignAlloc(size_t size, size_t align, const char* filename, int line)
	{
		void* ptr = allocator.AlignAllocate(size, align);
		MemoryTracker::Get().RecordAlloc(ptr, size, filename, line);
		return ptr;
	}

	void* Memory::AlignRealloc(void* ptr, size_t newBytes, size_t align, const char* filename, int line)
	{
		ptr = allocator.AlignReallocate(ptr, newBytes, align);
		MemoryTracker::Get().RecordRealloc(ptr, newBytes, filename, line);
		return ptr;
	}

	void Memory::AlignFree(void* ptr)
	{
		MemoryTracker::Get().RecordFree(ptr);
		allocator.AlignFree(ptr);
	}

#else
	void* Memory::Alloc(size_t size)
	{
		return allocator.Allocate(size);
	}

	void* Memory::Realloc(void* ptr, size_t newBytes)
	{
		return allocator.Reallocate(ptr, newBytes);
	}

	void Memory::Free(void* ptr)
	{
		return allocator.Free(ptr);
	}

	void* Memory::AlignAlloc(size_t size, size_t align)
	{
		return allocator.AlignAllocate(size, align);
	}

	void* Memory::AlignRealloc(void* ptr, size_t newBytes, size_t align)
	{
		return allocator.AlignReallocate(ptr, newBytes, align);
	}

	void Memory::AlignFree(void* ptr)
	{
		return allocator.AlignFree(ptr);
	}
#endif

	void Memory::Memmove(void* dst, void* src, size_t size)
	{
		memmove_s(dst, size, src, size);
	}

	void Memory::Memcpy(void* dst, void* src, size_t size)
	{
		memcpy_s(dst, size, src, size);
	}

}