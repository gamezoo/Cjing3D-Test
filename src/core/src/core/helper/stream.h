#pragma once

#include "core\common\definitions.h"
#include "core\memory\memory.h"
#include "core\string\string.h"

namespace Cjing3D
{
	class MemoryStream
	{
	private:
		DefaultAllocator mAllocator;
		U8* mBuffer = nullptr;
		U32 mCapacity = 0;
		U32 mOffset = 0;

	public:
		MemoryStream() = default;
		MemoryStream(void* buffer, U32 size) : mBuffer((U8*)buffer), mOffset(0), mCapacity(size) {}
		MemoryStream(MemoryStream&& rhs)
		{
			std::swap(mAllocator, rhs.mAllocator);
			std::swap(mBuffer, rhs.mBuffer);
			std::swap(mOffset, rhs.mOffset);
			std::swap(mCapacity, rhs.mCapacity);
		}
		MemoryStream(const MemoryStream& rhs)
		{
			mAllocator = rhs.mAllocator;
			mOffset = rhs.mOffset;
			mCapacity = rhs.mCapacity;
			if (rhs.mCapacity > 0)
			{
				mBuffer = (U8*)CJING_ALLOCATOR_MALLOC(mAllocator, rhs.mCapacity);
				Memory::Memcpy(mBuffer, rhs.mBuffer, rhs.mCapacity);
			}
		}
		
		void operator=(MemoryStream&& rhs)
		{
			std::swap(mAllocator, rhs.mAllocator);
			std::swap(mBuffer, rhs.mBuffer);
			std::swap(mOffset, rhs.mOffset);
			std::swap(mCapacity, rhs.mCapacity);
		}
		
		void operator=(const MemoryStream& rhs)
		{
			if (mBuffer != nullptr) 
			{
				CJING_ALLOCATOR_FREE(mAllocator, mBuffer);
				mBuffer = nullptr;
			}

			mAllocator = rhs.mAllocator;
			mOffset = rhs.mOffset;
			mCapacity = rhs.mCapacity;
			if (rhs.mCapacity > 0)
			{
				mBuffer = (U8*)CJING_ALLOCATOR_MALLOC(mAllocator, rhs.mCapacity);
				Memory::Memcpy(mBuffer, rhs.mBuffer, rhs.mCapacity);
			}
		}

		void Resize(U32 size)
		{
			mOffset = size;
			if (mOffset <= mCapacity) {
				return;
			}

			U8* tempBuf = (U8*)CJING_ALLOCATOR_MALLOC(mAllocator, mOffset);
			Memory::Memcpy(tempBuf, mBuffer, mCapacity);
			CJING_ALLOCATOR_FREE(mAllocator, mBuffer);
			mBuffer = tempBuf;
			mCapacity = mOffset;
		}

		void Reserve(U32 capacity)
		{
			if (capacity <= mCapacity) {
				return;
			}

			U8* tempBuf = (U8*)CJING_ALLOCATOR_MALLOC(mAllocator, capacity);
			Memory::Memcpy(tempBuf, mBuffer, mCapacity);
			CJING_ALLOCATOR_FREE(mAllocator, mBuffer);
			mBuffer = tempBuf;
			mCapacity = mOffset;
		}

		void Clear() {
			mOffset = 0;
		}

		void Free()
		{
			if (mBuffer) 
			{
				CJING_ALLOCATOR_FREE(mAllocator, mBuffer);
				mBuffer = nullptr;
			}
			mOffset = 0;
			mCapacity = 0;

		}

		bool Empty()const { return mOffset == 0; }

		void WriteString(const char* string)
		{
			if (string && string[0] != '\0') {
				Write(string, StringLength(string));
			}
			else {
				Write((U32)0);
			}
		}

		bool Write(const void* data, U32 size)
		{
			if (size <= 0 || data == nullptr) {
				return false;
			}

			if (mOffset + size > mCapacity) {
				Reserve(mCapacity * 2);
			}
			Memory::Memcpy(mBuffer + mOffset, data, size);
			mOffset += size;
			return true;
		}

		template<typename T>
		void Write(const T& value)
		{
			Write(&value, sizeof(T));
		}
	};
}