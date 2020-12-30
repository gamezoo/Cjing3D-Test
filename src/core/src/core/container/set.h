#pragma once

#include "hashMap.h"

namespace Cjing3D
{
	template<typename KeyT, typename Hasher = Hasher<KeyT>>
	class Set : public HashMap<KeyT, KeyT, Hasher>
	{
	public:
		static const U32 DEFAULT_SIZE = 16;
		
		Set(U32 size = DEFAULT_SIZE) :
			HashMap<KeyT, KeyT, Hasher>(size)
		{}

		KeyT* insert(KeyT key)
		{
			if (auto ret = find(key))
			{
				*ret = key;
				return ret;
			}

			if ((++this->mSize) >= this->mCapacityThreshold) {
				this->Grow();
			}
			return InsertImpl(HashKey(key), std::move(key), std::move(key));
		}
	};
}