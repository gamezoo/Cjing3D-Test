#include "handle.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
	Handle Handle::INVALID_HANDLE;

	HandleAllocator::HandleAllocator(I32 typeCount) :
		mMaxTypeCount(typeCount)
	{
		I32 maxMagic = Handle::MAX_INDEX * Handle::MAX_TYPE;
		mMagicIDs.resize(maxMagic);
		for (I32 i = 0; i < maxMagic; i++) {
			mMagicIDs[i] = 1;
		}
	}

	HandleAllocator::~HandleAllocator()
	{
	}

	Handle HandleAllocator::Alloc(I32 type)
	{
		Debug::CheckAssertion(type >= 0 && type < mMaxTypeCount);
		HandleTypeData& data = mHandleTypeDatas[type];

		Handle ret;
		bool success = false;
		if (data.mFreeList.size() > 0)
		{
			ret.mIndex = data.mFreeList.back();
			ret.mType = type;
			data.mFreeList.pop();
			success = true;
		}
		else
		{
			if (data.mAllocated.size() < Handle::MAX_INDEX)
			{
				data.mAllocated.push(0);
				ret.mIndex = data.mAllocated.size() - 1;
				ret.mType = type;
				success = true;
			}
		}

		if (success)
		{
			Debug::CheckAssertion(ret.mIndex < data.mAllocated.size());
			data.mAllocated[ret.mIndex] = 1;
			ret.mMagic = GetMagicID(ret.mIndex, ret.mType);
		}
		return ret;
	}

	void HandleAllocator::Free(Handle handle)
	{
		if (!IsValid(handle)) {
			return;
		}
		HandleTypeData& data = mHandleTypeDatas[handle.mType];

		auto& magicID = GetMagicID(handle.mIndex, handle.mType);
		if (magicID >= Handle::MAX_MAGIC - 1) {
			magicID = 0;
		}

		// magic inc, to validate new handle and old handle
		magicID++;

		data.mAllocated[handle.mIndex] = 0;
		data.mFreeList.push(handle.mIndex);
	}

	I32 HandleAllocator::GetTotalHandleCount(I32 type)
	{
		Debug::CheckAssertion(type >= 0 && type < mMaxTypeCount);
		HandleTypeData& data = mHandleTypeDatas[type];
		I32 count = 0;
		for (auto isAllocated : data.mAllocated) 
		{
			if (isAllocated) {
				count++;
			}
		}
		return count;
	}

	I32 HandleAllocator::GetMaxHandleCount(I32 type)
	{
		Debug::CheckAssertion(type >= 0 && type < mMaxTypeCount);
		HandleTypeData& data = mHandleTypeDatas[type];
		return data.mAllocated.size();
	}

	bool HandleAllocator::IsValid(Handle handle)
	{
		return mMagicIDs[handle.mIndex + handle.mType * Handle::MAX_INDEX] == handle.mMagic;
	}

	bool HandleAllocator::IsAllocated(Handle handle)const
	{
		const HandleTypeData& data = mHandleTypeDatas[handle.mType];
		return data.mAllocated[handle.mIndex] != 0;
	}

	bool HandleAllocator::IsAllocated(I32 type, I32 index)const
	{
		Debug::CheckAssertion(type >= 0 && type < mMaxTypeCount);
		const HandleTypeData& data = mHandleTypeDatas[type];
		return data.mAllocated[index] != 0;
	}

	U16& HandleAllocator::GetMagicID(U32 index, U32 type)
	{
		return mMagicIDs[index + type * Handle::MAX_INDEX];
	}
}