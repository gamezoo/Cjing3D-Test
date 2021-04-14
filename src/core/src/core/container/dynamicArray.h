#pragma once

#include "core\common\definitions.h"
#include "core\memory\memory.h"

#include <initializer_list>

namespace Cjing3D
{
	template<typename T>
	class DynamicArray
	{
	private:
		U32 mCapacity = 0;
		U32 mSize = 0;
		T* mData = nullptr;

		void Grow()
		{
			U32 newCapacity = mCapacity == 0 ? 4 : mCapacity * 2;
			reserve(newCapacity);
		}

		void CallDestructors(T* begin, T* end)
		{
			for (; begin < end; ++begin) {
				begin->~T();
			}
		}

		void UninitialiedMove(T* dst, T* src, U32 count)
		{
			for (U32 i = count - 1; i < count; i--) {
				new (dst + i) T(static_cast<T&&>(src[i]));
			}
		}

		void resizeImpl(U32 capacity)
		{
			U32 copySize = capacity < mSize ? capacity : mSize;
			T* newData = nullptr;
			if (capacity > 0)
			{
				if constexpr (__is_trivially_copyable(T)) {
					newData = (T*)CJING_ALLOCATOR_REMALLOC_ALIGN(mAllocator, mData, capacity * sizeof(T), alignof(T));
				}
				else
				{
					newData = (T*)CJING_ALLOCATOR_MALLOC_ALIGN(mAllocator, capacity * sizeof(T), alignof(T));
					UninitialiedMove(newData, mData, copySize);
					CallDestructors(mData, mData + copySize);
				}
			}

			if (copySize < mSize)
			{
				if constexpr (!__is_trivially_copyable(T)) 
				{
					CallDestructors(mData + copySize, mData + mSize);
					CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mData);
				}
			}

			mData = newData;
			mSize = copySize;
			mCapacity = capacity;
		}


	public:
		DynamicArray() = default;
		explicit DynamicArray(U32 size)
		{
			resize(size);
		}
		~DynamicArray()
		{
			CallDestructors(mData, mData + mSize);
			CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mData);
		}

		DynamicArray(const DynamicArray& rhs)
		{
			resizeImpl(rhs.mSize);
			mSize = rhs.mSize;
			for (U32 i = 0; i < mSize; ++i) {
				new((char*)(mData + i)) T(rhs.mData[i]);
			}
		}

		DynamicArray(DynamicArray&& rhs)
		{
			swap(rhs);
		}

		DynamicArray(std::initializer_list<T> _list)
		{
			resizeImpl(_list.size());
			mSize = _list.size();
			for (U32 i = 0; i < mSize; ++i) {
				new((char*)(mData + i)) T(*(_list.begin() + i));
			}
		}

		void operator= (const DynamicArray& rhs)
		{
			if (rhs.mSize != mSize) {
				resizeImpl(rhs.mSize);
			}

			CallDestructors(mData, mData + mSize);

			mSize = rhs.mSize;
			for (U32 i = 0; i < mSize; ++i) {
				new((char*)(mData + i)) T(rhs.mData[i]);
			}
		}

		void operator= (DynamicArray&& rhs)
		{
			if (this != &rhs)
			{
				CallDestructors(mData, mData + mSize);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mData);

				mData = rhs.mData;
				mCapacity = rhs.mCapacity;
				mSize = rhs.mSize;

				rhs.mData = nullptr;
				rhs.mCapacity = 0;
				rhs.mSize = 0;
			}
		}

		T* begin() const { return mData; }
		T* end() const { return mData ? mData + mSize : nullptr; }

		void swap(DynamicArray<T>& rhs)
		{
			std::swap(mCapacity, rhs.mCapacity);
			std::swap(mSize, rhs.mSize);
			std::swap(mData, rhs.mData);
		}

		template <typename Comparator>
		void removeDuplicates(Comparator equals)
		{
			if (mSize == 0) return;
			for (U32 i = 0; i < mSize - 1; ++i)
			{
				for (U32 j = i + 1; j < mSize; ++j)
				{
					if (equals(mData[i], mData[j]))
					{
						swapAndPop(j);
						--j;
					}
				}
			}
		}

		void removeDuplicates()
		{
			if (mSize == 0) return;
			for (U32 i = 0; i < mSize - 1; ++i)
			{
				for (U32 j = i + 1; j < mSize; ++j)
				{
					if (mData[i] == mData[j])
					{
						swapAndPop(j);
						--j;
					}
				}
			}
		}

		void free()
		{
			clear();
			CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mData);
			mCapacity = 0;
			mData = nullptr;
		}

		template <typename F>
		int find(F predicate) const
		{
			for (U32 i = 0; i < mSize; ++i)
			{
				if (predicate(mData[i]))
				{
					return i;
				}
			}
			return -1;
		}

		void Fill(const T& value)
		{
			CallDestructors(mData, mData + mSize);
			for (U32 i = 0; i < mSize; ++i) {
				new((char*)(mData + i)) T(value);
			}
		}

		template <typename R>
		int indexOf(R item) const
		{
			for (U32 i = 0; i < mSize; ++i)
			{
				if (mData[i] == item)
				{
					return i;
				}
			}
			return -1;
		}

		int indexOf(const T& item) const
		{
			for (U32 i = 0; i < mSize; ++i)
			{
				if (mData[i] == item)
				{
					return i;
				}
			}
			return -1;
		}

		template <typename F>
		void eraseItems(F predicate)
		{
			for (U32 i = mSize - 1; i != 0xffFFffFF; --i)
			{
				if (predicate(mData[i]))
				{
					erase(i);
				}
			}
		}

		void swapAndPopItem(const T& item)
		{
			for (U32 i = 0; i < mSize; ++i)
			{
				if (mData[i] == item)
				{
					swapAndPop(i);
					return;
				}
			}
		}

		void swapAndPop(U32 index)
		{
			if (index >= 0 && index < mSize)
			{
				mData[index].~T();
				if (index != mSize - 1)
				{
					Memory::Memmove(mData + index, mData + mSize - 1, sizeof(T));
				}
				--mSize;
			}
		}

		void eraseItem(const T& item)
		{
			for (U32 i = 0; i < mSize; ++i)
			{
				if (mData[i] == item)
				{
					erase(i);
					return;
				}
			}
		}

		void insert(U32 index, const T& value)
		{
			if (mSize == mCapacity) {
				Grow();
			}

			Memory::Memmove(mData + index + 1, mData + index, sizeof(T) * (mSize - index));
			new(&mData[index]) T(value);
			++mSize;
		}

		void insert(const T* begin, const T* end)
		{
			I32 num = (I32)(end - begin);
			if (num <= 0) {
				return;
			}
			if (mSize + num > mCapacity) {
				reserve((mSize + num) * 2);
			}

			for (const T* it = begin; it != end; ++it)
			{
				new((char*)(mData + size)) T(*it);
				++size;
			}
		}

		void erase(U32 index)
		{
			if (index < mSize)
			{
				mData[index].~T();
				if (index < mSize - 1) {
					Memory::Memcpy(mData + index, mData + index + 1, sizeof(T) * (mSize - index - 1));
				}
				--mSize;
			}
		}

		T* erase(T* it)
		{
			assert(it >= begin() && it < end());
			it->~T();
			auto index = it - begin();
			if (index < mSize - 1) {
				Memory::Memcpy(mData + index, mData + index + 1, sizeof(T) * (mSize - index - 1));
			}
			--mSize;
			return it;
		}

		void erase(T* beginIt, T* endIt)
		{
			if (beginIt == endIt) {
				return;
			}

			assert(beginIt >= begin() && beginIt < end());
			assert(endIt >= begin() && endIt <= end());

			CallDestructors(beginIt, endIt);
			if (endIt - begin() < mSize - 1) {
				Memory::Memcpy(beginIt, endIt + 1, sizeof(T) * (end() - endIt - 1));
			}
			mSize -= (U32)(endIt - beginIt);
		}

		void push(const T& value)
		{
			U32 size = mSize;
			if (size == mCapacity) {
				Grow();
			}

			new((char*)(mData + size)) T(value);
			++size;
			mSize = size;
		}

		void push(T&& value)
		{
			U32 size = mSize;
			if (size == mCapacity) {
				Grow();
			}
			new((char*)(mData + size)) T(static_cast<T&&>(value));
			++size;
			mSize = size;
		}

		template <typename _Ty> struct remove_reference { typedef _Ty type; };
		template <typename _Ty> struct remove_reference<_Ty&> { typedef _Ty type; };
		template <typename _Ty> struct remove_reference<_Ty&&> { typedef _Ty type; };

		template <typename _Ty> 
		_Ty&& myforward(typename remove_reference<_Ty>::type& _Arg)
		{
			return (static_cast<_Ty&&>(_Arg));
		}

		template <typename... Args> 
		T& emplace(Args&&... args)
		{
			if (mSize == mCapacity) {
				Grow();
			}

			new((char*)(mData + mSize)) T(myforward<Args>(args)...);
			++mSize;
			return mData[mSize - 1];
		}

		template <typename... Args>
		T& emplaceAt(U32 idx, Args&&... args)
		{
			if (mSize == mCapacity) {
				Grow();
			}

			Memory::Memmove(&mData[idx + 1], &mData[idx], sizeof(mData[idx]) * (mSize - idx));

			new((char*)(mData + idx)) T(myforward<Args>(args)...);
			++mSize;
			return mData[idx];
		}

		bool empty() const { return mSize == 0; }

		void clear()
		{
			CallDestructors(mData, mData + mSize);
			mSize = 0;
		}

		const T& back() const { return mData[mSize - 1]; }
		T& back() { return mData[mSize - 1]; }

		void pop()
		{
			if (mSize > 0)
			{
				mData[mSize - 1].~T();
				--mSize;
			}
		}

		void pop(I32 count)
		{
			if (mSize > 0 && count > 0)
			{
				count = std::min((I32)mSize, count);
				for (int i = 1; i <= count; i++) {
					mData[mSize - i].~T();
				}
				mSize -= count;
			}
		}

		void resize(U32 size)
		{
			if (size > mCapacity) {
				reserve(size);
			}

			for (U32 i = mSize; i < size; ++i) {
				new((char*)(mData + i)) T;
			}

			CallDestructors(mData + size, mData + mSize);
			mSize = size;
		}

		void reserve(U32 capacity)
		{
			if (capacity > mCapacity)
			{
				if constexpr (__is_trivially_copyable(T)) {
					mData = (T*)CJING_ALLOCATOR_REMALLOC_ALIGN(mAllocator, mData, capacity * sizeof(T), alignof(T));
				}
				else
				{
					T* newData = (T*)CJING_ALLOCATOR_MALLOC_ALIGN(mAllocator, capacity * sizeof(T), alignof(T));
					UninitialiedMove(newData, mData, mSize);
					CallDestructors(mData, mData + mSize);
					CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mData);
					mData = newData;
				}
				mCapacity = capacity;
			}
		}

		T* data()
		{
			return mData;
		}


		const T* data()const
		{
			return mData;
		}

		const T& operator[](U32 index) const
		{
			assert(index < mSize);
			return mData[index];
		}

		T& operator[](U32 index)
		{
			assert(index < mSize);
			return mData[index];
		}

		U32 byte_size() const { return mSize * sizeof(T); }
		int size() const { return mSize; }
		U32 capacity() const { return mCapacity; }

		ContainerAllocator mAllocator;
	};
}