#pragma once

#include "mem_def.h"
#include "allocator.h"
#include "memTracker.h"

namespace Cjing3D
{
	struct NewPlaceHolder {};

#if (CJING_MEMORY_ALLOCATOR == CJING_MEMORY_ALLOCATOR_DEFAULT)
	using MemoryAllocator = DefaultAllocator;
#endif

#if (CJING_CONTAINER_ALLOCATOR == CJING_MEMORY_ALLOCATOR_DEFAULT)
	using ContainerAllocator = DefaultAllocator;
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

#ifdef  CJING_MEMORY_TRACKER
#define CJING_NEW(T) new (Cjing3D::NewPlaceHolder(), Cjing3D::Memory::Alloc(sizeof(T), __FILE__, __LINE__)) T
#define CJING_DELETE(ptr) Cjing3D::Memory::ObjectConstruct(ptr); Cjing3D::Memory::Free(ptr);
#define CJING_NEW_ARR(T, count) Cjing3D::Memory::ArrayConstructFunc(static_cast<T*>(Cjing3D::Memory::Alloc(sizeof(T) * count, __FILE__, __LINE__)), count)
#define CJING_DELETE_ARR(ptr, count) Cjing3D::Memory::ArrayDeconstructFunc(ptr, count); Cjing3D::Memory::Free(ptr);

#define CJING_MALLOC(size)  Cjing3D::Memory::Alloc(size, __FILE__, __LINE__)
#define CJING_MALLOC_ALIGN(size, align)  Cjing3D::Memory::AlignAlloc(size, align, __FILE__, __LINE__)
#define CJING_REMALLOC(ptr, size)  Cjing3D::Memory::Realloc(ptr, size, __FILE__, __LINE__)
#define CJING_REMALLOC_ALIGN(ptr, size, align)  Cjing3D::Memory::AlignRealloc(ptr, size, align, __FILE__, __LINE__)
#define CJING_FREE(ptr) Cjing3D::Memory::Free(ptr);
#define CJING_FREE_ALIGN(ptr) Cjing3D::Memory::AlignFree(ptr);

#define CJING_ALLOCATOR_MALLOC(allocator, size)  allocator.Allocate(size, __FILE__, __LINE__)
#define CJING_ALLOCATOR_MALLOC_ALIGN(allocator, size, align)  allocator.AlignAllocate(size, align, __FILE__, __LINE__)
#define CJING_ALLOCATOR_REMALLOC(allocator, ptr, size)  allocator.Reallocate(ptr, size, __FILE__, __LINE__)
#define CJING_ALLOCATOR_REMALLOC_ALIGN(allocator, ptr, size, align)  allocator.AlignReallocate(ptr, size, align, __FILE__, __LINE__)
#define CJING_ALLOCATOR_FREE(allocator, ptr) allocator.Free(ptr);
#define CJING_ALLOCATOR_FREE_ALIGN(allocator, ptr) allocator.AlignFree(ptr); 
#define CJING_ALLOCATOR_NEW(allocator, T) new (Cjing3D::NewPlaceHolder(), allocator.Allocate(sizeof(T), __FILE__, __LINE__)) T
#define CJING_ALLOCATOR_DELETE(allocator, ptr) Cjing3D::Memory::ObjectConstruct(ptr); allocator.Free(ptr);

#else
#define CJING_NEW(T) new (Cjing3D::NewPlaceHolder(), Cjing3D::Memory::Alloc(sizeof(T))) T
#define CJING_DELETE(ptr) Cjing3D::Memory::ObjectConstruct(ptr); Cjing3D::Memory::Free(ptr);
#define CJING_NEW_ARR(T, count) Cjing3D::Memory::ArrayConstructFunc(static_cast<T*>(Cjing3D::Memory::Alloc(sizeof(T) * count)), count)
#define CJING_DELETE_ARR(ptr, count) Cjing3D::Memory::ArrayDeconstructFunc(ptr, count); Cjing3D::Memory::Free(ptr);

#define CJING_MALLOC(size)  Cjing3D::Memory::Alloc(size)
#define CJING_MALLOC_ALIGN(size, align)  Cjing3D::Memory::AlignAlloc(size, align)
#define CJING_REMALLOC(ptr, size)  Cjing3D::Memory::Realloc(ptr, size)
#define CJING_REMALLOC_ALIGN(ptr, size, align)  Cjing3D::Memory::AlignRealloc(ptr, size, align)
#define CJING_FREE(ptr) Cjing3D::Memory::Free(ptr);
#define CJING_FREE_ALIGN(ptr) Cjing3D::Memory::AlignFree(ptr); 

#define CJING_ALLOCATOR_MALLOC(allocator, size)  allocator.Allocate(size)
#define CJING_ALLOCATOR_MALLOC_ALIGN(allocator, size, align)  allocator.AlignAllocate(size, align)
#define CJING_ALLOCATOR_REMALLOC(allocator, ptr, size)  allocator.Reallocate(ptr, size)
#define CJING_ALLOCATOR_REMALLOC_ALIGN(allocator, ptr, size, align)  allocator.AlignReallocate(ptr, size, align)
#define CJING_ALLOCATOR_FREE(allocator, ptr) allocator.Free(ptr);
#define CJING_ALLOCATOR_FREE_ALIGN(allocator, ptr) allocator.AlignFree(ptr); 
#define CJING_ALLOCATOR_NEW(allocator, T) new (Cjing3D::NewPlaceHolder(), allocator.Allocate(size)) T
#define CJING_ALLOCATOR_DELETE(allocator, ptr) Cjing3D::Memory::ObjectConstruct(ptr); allocator.Free(ptr);


#endif

#define CJING_SAFE_DELETE(ptr) { if (ptr) { CJING_DELETE(ptr); ptr = nullptr; } }
#define CJING_SAFE_DELETE_ARR(ptr, count) { if (ptr) { CJING_DELETE_ARR(ptr, count); ptr = nullptr; } }
#define CJING_SAFE_FREE(ptr) { if (ptr) { CJING_FREE(ptr); ptr = nullptr; } }

// smart pointer
namespace Cjing3D
{
#ifdef USE_STL_SMART_POINTER
	template<typename T>
	inline void SmartPointDeleter(T* v)
	{
		CJING_DELETE(v);
	}

	template<typename T>
	using SharedPtr = std::shared_ptr<T>;

	template< typename T>
	SharedPtr<T> CJING_SHARED(T* ptr)
	{
		return SharedPtr<T>(ptr, SmartPointDeleter<T>);
	}

	template< typename T, typename... Args >
	SharedPtr<T> CJING_MAKE_SHARED(Args&&... args)
	{
		return SharedPtr<T>(
			CJING_NEW(T){ std::forward<Args>(args)... },
			SmartPointDeleter<T>
		);
	}

	template<typename T>
	using ENABLE_SHARED_FROM_THIS = std::enable_shared_from_this<T>;

	template<typename T>
	using UniquePtr = std::unique_ptr<T, decltype(SmartPointDeleter<T>)*>;

	template<typename T>
	UniquePtr<T> NULL_UNIQUE()
	{
		static UniquePtr<T> nullUniquePtr(nullptr, SmartPointDeleter<T>);
		return nullUniquePtr;
	}

	template< typename T>
	UniquePtr<T> CJING_UNIQUE(T* ptr)
	{
		return UniquePtr<T>(ptr, SmartPointDeleter<T>);
	}

	template< typename T, typename... Args >
	UniquePtr<T> CJING_MAKE_UNIQUE(Args&&... args)
	{
		return UniquePtr<T>(
			CJING_NEW(T) { std::forward<Args>(args)... },
			SmartPointDeleter<T>
		);
	}
#endif
}