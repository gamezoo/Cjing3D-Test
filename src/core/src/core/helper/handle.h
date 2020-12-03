#pragma once

#include "core\common\definitions.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D
{
	class Handle
	{
	public:
		Handle() : mValue(0) {}

		U32 GetIndex()const {
			return mIndex;
		}
		U32 GetType() {
			return mType;
		}

		explicit operator bool() const { 
			return mValue != 0; 
		}
		bool operator<(const Handle other) const { 
			return mValue < other.mValue;
		}
		bool operator>(const Handle other) const { 
			return mValue > other.mValue;
		}
		bool operator<=(const Handle other) const { 
			return mValue <= other.mValue;
		}
		bool operator>=(const Handle other) const { 
			return mValue >= other.mValue;
		}
		bool operator==(const Handle other) const { 
			return mValue == other.mValue;
		}
		bool operator!=(const Handle other) const {
			return mValue != other.mValue;
		}

		static const I32 MAX_INDEX = (1 << 16) - 1;
		static const I32 MAX_MAGIC = (1 << 12) - 1;
		static const I32 MAX_TYPE  = (1 << 4)  - 1;

		static Handle INVALID_HANDLE;

	private:
		friend class HandleAllocator;

		union
		{
			struct 
			{
				U32 mIndex : 16;
				U32 mMagic : 12;
				U32 mType  : 4;
			};
			U32 mValue;
		};
	};
	
	class HandleAllocator
	{
	public:
		HandleAllocator(I32 typeCount);
		~HandleAllocator();

		Handle Alloc(I32 type);
		void Free(Handle handle);

		I32 GetTotalHandleCount(I32 type);
		I32 GetMaxHandleCount(I32 type);
		bool IsValid(Handle handle);
		bool IsAllocated(Handle handle);

	private:
		U16& GetMagicID(U32 index, U32 type);

		struct HandleTypeData
		{
			DynamicArray<U32> mFreeList;
			DynamicArray<U8>  mAllocated;	// 0:free, 1:allocated
		};
		DynamicArray<HandleTypeData> mHandleTypeDatas;

		// MAX_TYPE * MAX_INDEX magic ids for validating handle
		DynamicArray<U16> mMagicIDs;	
		I32 mMaxTypeCount;
	};
}