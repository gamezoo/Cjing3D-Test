#pragma once

#include "core\memory\allocator.h"
#include "core\memory\memTracker.h"

#define CJING_MEMORY_ALLOCATOR	CJING_MEMORY_ALLOCATOR_DEFAULT

namespace Cjing3D
{
	struct NewPlaceHolder {};

#if (CJING_MEMORY_ALLOCATOR == CJING_MEMORY_ALLOCATOR_DEFAULT)
	using MemoryAllocator = DefaultAllocator;
#endif

	class Memory
	{
	public:
#ifdef CJING_MEMORY_TRACKER
		static void* Alloc(size_t size, const char* filename, int line);
		static void* Realloc(void* ptr, size_t newBytes, const char* filename, int line);
		static void  Free(void* ptr);
		static void* AlignAlloc(size_t size, size_t align, const char* filename, int line);
		static void* AlignRealloc(void* ptr, size_t newBytes, size_t align, const char* filename, int line);
		static void  AlignFree(void* ptr);
#else
		static void* Alloc(size_t size);
		static void* Realloc(void* ptr, size_t newBytes);
		static void  Free(void* ptr);
		static void* AlignAlloc(size_t size, size_t align);
		static void* AlignRealloc(void* ptr, size_t newBytes, size_t align);
		static void  AlignFree(void* ptr);

#endif
		static void  Memmove(void* dst, const void* src, size_t size);
		static void  Memcpy(void* dst, const void* src, size_t size);
		static void  Memset(void* dst, int c, int count);

		template<typename T>
		static void ObjectConstruct(T* ptr)
		{
			if (ptr == nullptr) {
				return;
			}
			if (!__has_trivial_destructor(T)) {
				ptr->~T();
			}
		}

		template<typename T>
		static T* ArrayConstructFunc(T* mem, size_t num)
		{
			T* elems = (T*)mem;
			if (!__has_trivial_constructor(T)) {
				for (size_t i = 0; i < num; i++) {
					new(&elems[i]) T();
				}
			}
			return elems;
		}

		template<typename T>
		static void ArrayDeconstructFunc(T* ptrArr, size_t num)
		{
			uint64_t* mem = (uint64_t*)ptrArr;
			if (mem == nullptr) {
				return;
			}
			if (!__has_trivial_destructor(T)) 
			{
				for (size_t i = 0; i < num; i++) {
					ptrArr[i].~T();
				}
			}
		}
	};
}

inline void* operator new(size_t size, Cjing3D::NewPlaceHolder, void* pointer) {
	return pointer;
}
inline void operator delete(void*, Cjing3D::NewPlaceHolder, void*) { }

#ifdef CJING_MEMORY_TRACKER
#define CJING_NEW(T) new (Cjing3D::NewPlaceHolder(), Cjing3D::Memory::Alloc(sizeof(T), __FILE__, __LINE__)) T
#define CJING_DELETE(ptr) Cjing3D::Memory::ObjectConstruct(ptr); Cjing3D::Memory::Free(ptr);
#define CJING_NEW_ARR(T, count) Cjing3D::Memory::ArrayConstructFunc(static_cast<T*>(Cjing3D::Memory::Alloc(sizeof(T) * count, __FILE__, __LINE__)), count)
#define CJING_DELETE_ARR(ptr, count) Cjing3D::Memory::ArrayDeconstructFunc(ptr, count); Cjing3D::Memory::Free(ptr);

#define CJING_MALLOC(size)  Cjing3D::Memory::Alloc(size, __FILE__, __LINE__);
#define CJING_MALLOC_ALIGN(size, align)  Cjing3D::Memory::AlignAlloc(size, align, __FILE__, __LINE__);
#define CJING_FREE(ptr) Cjing3D::Memory::Free(ptr);
#define CJING_FREE_ALIGN(ptr) Cjing3D::Memory::AlignFree(ptr); 

#else
#define CJING_NEW(T) new (Cjing3D::NewPlaceHolder(), Cjing3D::Memory::Alloc(sizeof(T))) T
#define CJING_DELETE(ptr) Cjing3D::Memory::ObjectConstruct(ptr); Cjing3D::Memory::Free(ptr);
#define CJING_NEW_ARR(T, count) Cjing3D::Memory::ArrayConstructFunc(static_cast<T*>(Cjing3D::Memory::Alloc(sizeof(T) * count)), count)
#define CJING_DELETE_ARR(ptr, count) Cjing3D::Memory::ArrayDeconstructFunc(ptr, count); Cjing3D::Memory::Free(ptr);

#define CJING_MALLOC(size)  Cjing3D::Memory::Alloc(size);
#define CJING_MALLOC_ALIGN(size, align)  Cjing3D::Memory::AlignAlloc(size, align);
#define CJING_FREE(ptr) Cjing3D::Memory::Free(ptr);
#define CJING_FREE_ALIGN(ptr) Cjing3D::Memory::AlignFree(ptr); 

#endif

#define CJING_SAFE_DELETE(ptr) { if (ptr) { CJING_DELETE(ptr); ptr = nullptr; } }
#define CJING_SAFE_FREE(ptr) { if (ptr) { CJING_FREE(ptr); ptr = nullptr; } }
