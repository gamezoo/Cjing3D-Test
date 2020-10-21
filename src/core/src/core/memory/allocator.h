#pragma once

#include "mem_def.h"

#include <memory>

namespace Cjing3D
{
	class IAllocator
	{
	public:
		IAllocator() {};
		virtual ~IAllocator() {};

#ifdef CJING_MEMORY_TRACKER
		virtual void* Allocate(size_t size, const char* filename, int line) = 0;
		virtual void* Reallocate(void* ptr, size_t newBytes, const char* filename, int line) = 0;
		virtual void  Free(void* ptr) = 0;
		virtual void* AlignAllocate(size_t size, size_t align, const char* filename, int line) = 0;
		virtual void* AlignReallocate(void* ptr, size_t newBytes, size_t align, const char* filename, int line) = 0;
		virtual void  AlignFree(void* ptr) = 0;
#else
		virtual void* Allocate(size_t size) = 0;
		virtual void* Reallocate(void* ptr, size_t newSize) = 0;
		virtual void  Free(void* ptr) = 0;
		virtual void* AlignAllocate(size_t size, size_t align) = 0;
		virtual void* AlignReallocate(void* ptr, size_t newSize, size_t align) = 0;
		virtual void  AlignFree(void* ptr) = 0;
#endif

		// Get the maximum size of a single allocation
		virtual size_t GetMaxAllocationSize() = 0;

		template<typename T, typename... Args>
		T* New(Args&&... args)
		{
			void* mem = Allocate(sizeof(T), alignof(T));
			return new(mem) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void Delete(T* obj)
		{
			if (obj != nullptr)
			{
				if (!__has_trivial_destructor(T)) {
					obj->~T();
				}

				Free(obj);
			}
		}
	};

	class DefaultAllocator : public IAllocator
	{
	public:
		DefaultAllocator() {};
		virtual ~DefaultAllocator() {};

#ifdef CJING_MEMORY_TRACKER
		void* Allocate(size_t size, const char* filename, int line)override;
		void* Reallocate(void* ptr, size_t newBytes, const char* filename, int line)override;
		void  Free(void* ptr)override;
		void* AlignAllocate(size_t size, size_t align, const char* filename, int line)override;
		void* AlignReallocate(void* ptr, size_t newBytes, size_t align, const char* filename, int line)override;
		void  AlignFree(void* ptr)override;
#else
		void* Allocate(size_t size)override;
		void* Reallocate(void* ptr, size_t newSize)override;
		void  Free(void* ptr)override;
		void* AlignAllocate(size_t size, size_t align)override;
		void* AlignReallocate(void* ptr, size_t newSize, size_t align)override;
		void  AlignFree(void* ptr)override;
#endif

		// Get the maximum size of a single allocation
		size_t GetMaxAllocationSize()override;
	};
}