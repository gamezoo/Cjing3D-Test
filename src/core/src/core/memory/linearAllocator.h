#pragma once

#include "memory.h"

#include <cstdint>

namespace Cjing3D
{
	// simple linear, non thread safe
	class LinearAllocator
	{
	public:
		LinearAllocator() = default;
		~LinearAllocator()
		{
			FreeBuffer();
		}

		inline void FreeBuffer()
		{
			if (mBuffer != nullptr)
			{
				CJING_DELETE_ARR(mBuffer, mCapacity);
				mBuffer = nullptr;
				mCapacity = 0;
				mOffset = 0;
			}
		}

		inline void Reserve(size_t capacity)
		{
			if (mCapacity > 0 && mBuffer != nullptr)
			{
				CJING_DELETE_ARR(mBuffer, mCapacity);
				mCapacity = 0;
				mOffset = 0;
			}

			mBuffer = CJING_NEW_ARR(uint8_t, capacity);
			mCapacity = capacity;
		}

		inline uint8_t* Allocate(size_t size)
		{
			if (mOffset + size <= mCapacity)
			{
				uint8_t* data = &mBuffer[mOffset];
				mOffset += size;
				return data;
			}
			return nullptr;
		}

		inline void Free(size_t size)
		{
			size = std::min(mOffset, size);
			if (size > 0) {
				mOffset -= size;
			}
		}

		inline void Reset()
		{
			mOffset = 0;
		}

		inline size_t GetCapacity() const 
		{
			return mCapacity;
		}

		U8* GetBuffer() 
		{
			return mBuffer;
		}

		size_t GetOffset()const 
		{
			return mOffset;
		}

	private:
		U8* mBuffer = nullptr;
		size_t mCapacity = 0;
		size_t mOffset = 0;
	};
}