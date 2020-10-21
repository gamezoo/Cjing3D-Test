#pragma once

#include "allocator.h"

namespace Cjing3D
{
	class STLAllocPolicy
	{
	public:
		static void*  Allocate(size_t size);
		static void   Free(void* ptr);
		static size_t GetMaxAllocationSize();
	};

	template<typename T>
	struct STLAllocatorBase
	{
		typedef T value_type;
	};

	// custom stl allocator
	template<typename T, typename AllocatorT>
	class STLAllocator
	{
	public:
		typedef STLAllocatorBase<T>			Base;
		typedef typename Base::value_type	value_type;
		typedef value_type*					pointer;
		typedef const value_type*			const_pointer;
		typedef value_type&					reference;
		typedef const value_type&			const_reference;
		typedef size_t						size_type;
		typedef ptrdiff_t					difference_type;

		template<typename U>
		struct rebind
		{
			typedef STLAllocator<U, AllocatorT> other;
		};


		inline explicit STLAllocator() {}
		virtual ~STLAllocator() {}
		inline STLAllocator(STLAllocator const&) {}

		template <typename U>
		inline STLAllocator(STLAllocator<U, AllocatorT> const&) {}

		template <typename U, typename P>
		inline STLAllocator(STLAllocator<U, P> const&) {}

		////////////////////////////////////////////////////////////////

		size_type max_size() const throw()
		{
			return AllocatorT::GetMaxAllocationSize();
		}

		pointer allocate(size_type n, const_pointer = 0)
		{
			size_type sz = n * sizeof(T);
			pointer p = static_cast<pointer>(AllocatorT::Allocate(sz));
			return p;
		}

		void deallocate(pointer ptr, size_type)
		{
			AllocatorT::Free(ptr);
		}

		void construct(pointer ptr, const T& val)
		{
			new(static_cast<void*>(ptr)) T(val);
		}

		void destroy(pointer ptr)
		{
			if (!__has_trivial_destructor(T)) {
				ptr->~T();
			}
		}
	};

	template<typename T, typename T2, typename P>
	inline bool operator == (STLAllocator<T, P> const&, STLAllocator<T2, P> const&)
	{
		return true;
	}

	template<typename T, typename T2, typename P>
	inline bool operator != (STLAllocator<T, P> const&, STLAllocator<T2, P> const&)
	{
		return false;
	}

	template<typename T, typename P, typename OtherAllocator>
	inline bool operator == (STLAllocator<T, P> const&, OtherAllocator const&)
	{
		return false;
	}

	template<typename T, typename P, typename OtherAllocator>
	inline bool operator != (STLAllocator<T, P> const&, OtherAllocator const&)
	{
		return true;
	}
}