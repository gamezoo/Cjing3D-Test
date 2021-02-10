#include "memory.h"

namespace Cjing3D
{
	MemoryAllocator allocator;

#ifdef CJING_MEMORY_TRACKER
	void* Memory::Alloc(size_t size, const char* filename, int line)
	{
		return allocator.Allocate(size, filename, line);
	}

	void* Memory::Realloc(void* ptr, size_t newBytes, const char* filename, int line)
	{
		return allocator.Reallocate(ptr, newBytes, filename, line);
	}

	void Memory::Free(void* ptr)
	{
		allocator.Free(ptr);
	}

	void* Memory::AlignAlloc(size_t size, size_t align, const char* filename, int line)
	{
		return allocator.AlignAllocate(size, align, filename, line);
	}

	void* Memory::AlignRealloc(void* ptr, size_t newBytes, size_t align, const char* filename, int line)
	{
		return allocator.AlignReallocate(ptr, newBytes, align, filename, line);
	}

	void Memory::AlignFree(void* ptr)
	{
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

	void Memory::Memmove(void* dst, const void* src, size_t size)
	{
		memmove_s(dst, size, src, size);
	}

	void Memory::Memcpy(void* dst, const void* src, size_t size)
	{
		memcpy_s(dst, size, src, size);
	}

	void Memory::Memset(void* dst, int c, int count)
	{
		memset(dst, c, count);
	}

	void* Memory::StackAlloca(size_t size)
	{
		return alloca(size);
	}

}