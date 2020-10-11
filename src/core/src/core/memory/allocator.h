#pragma once

#include <memory>

#define CJING_MEMORY_ALLOCATOR_DEFAULT 1

namespace Cjing3D
{
	class IAllocator
	{
	public:
		IAllocator() {};
		virtual ~IAllocator() {};

		virtual void* Allocate(size_t size) = 0;
		virtual void* Reallocate(void* ptr, size_t newSize) = 0;
		virtual void  Free(void* ptr) = 0;
		virtual void* AlignAllocate(size_t size, size_t align) = 0;
		virtual void* AlignReallocate(void* ptr, size_t newSize, size_t align) = 0;
		virtual void  AlignFree(void* ptr) = 0;

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

		void* Allocate(size_t size)override;
		void* Reallocate(void* ptr, size_t newSize)override;
		void  Free(void* ptr)override;
		void* AlignAllocate(size_t size, size_t align)override;
		void* AlignReallocate(void* ptr, size_t newSize, size_t align)override;
		void  AlignFree(void* ptr)override;
	};
}