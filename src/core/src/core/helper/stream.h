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
		~MemoryStream()
		{
			if (mBuffer) {
				CJING_ALLOCATOR_FREE(mAllocator, mBuffer);
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
			mCapacity = capacity;
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
			if (string) {
				Write(string, StringLength(string) + 1);
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
				Reserve((mOffset + size) * 2);
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

		template<>
		void Write<bool>(const bool& value)
		{
			U8 v = value;
			Write(&v, sizeof(v));
		}

		const U8* data()const {
			return mBuffer;
		}
		U8* data() {
			return mBuffer;
		}

		U32 Size()const {
			return mOffset;
		}

		const U8* OffsetData()const {
			return mBuffer + mOffset;
		}

		U8* OffsetData() {
			return mBuffer + mOffset;
		}

		U32 GetCapacity()const {
			return mCapacity;
		}

		U32 GetRemainingCapacity()const {
			return mCapacity - mOffset;
		}

		void ConsumeSize(U32 size, bool bReset)
		{
			if (mOffset + size >= mCapacity)
			{
				mOffset = mCapacity;
				if (bReset) {
					Clear();
				}
				return;
			}
			mOffset += size;
		}
	};

	class InputMemoryStream
	{
	private:
		const U8* mBuffer = nullptr;
		U32 mSize = 0;
		U32 mOffset = 0;

	public:
		explicit InputMemoryStream(const MemoryStream& stream) : 
			mBuffer(stream.data()), mSize(stream.Size()) {}
		InputMemoryStream(const U8* data, U32 size) :
			mBuffer(data), mSize(size) {}

		bool Read(void* data, U32 size)
		{
			if (mOffset + size > mSize) {
				return false;
			}
			if (size > 0) {
				Memory::Memcpy(data, mBuffer + mOffset, size);
			}
			mOffset += size;
			return true;
		}

		bool Read(String& string)
		{
			string = ReadString();
			return true;
		}

		const char* ReadString()
		{
			const char* ret = (const char*)mBuffer + mOffset;
			const char* end = (const char*)mBuffer + mSize;
			while (mOffset < mSize && mBuffer[mOffset]) {
				++mOffset;
			}
			++mOffset;
			return ret;
		}

		template<typename T> 
		T Read() 
		{
			T v;
			Read(&v, sizeof(T));
			return v;
		}

		template<>
		bool Read<bool>()
		{
			U8 v;
			Read(&v, sizeof(v));
			return v != 0;
		}

		const U8* data()const {
			return mBuffer;
		}

		U32 Offset()const {
			return mOffset;
		}
	};
}