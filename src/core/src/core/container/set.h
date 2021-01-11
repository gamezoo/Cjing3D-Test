#pragma once

#include "hashMap.h"

namespace Cjing3D
{
	template<typename KeyT, typename Hasher = Hasher<KeyT>>
	class Set : public HashMap<KeyT, KeyT, Hasher>
	{
	public:
		static const U32 DEFAULT_SIZE = 16;
		using HashMapT = HashMap<KeyT, KeyT, Hasher>;
		
		class Iterator : public HashMapT::IteratorBase
		{
		public:
			Iterator(const HashMapT* parent, I32 pos) : HashMapT::IteratorBase(parent, pos){}

			KeyT operator*() {
				return this->mHashMap->GetKeyByIndex(this->mPos);
			}
		};

		class ConstIterator : public HashMapT::IteratorBase
		{
		public:
			ConstIterator(const HashMapT* parent, I32 pos) : HashMapT::IteratorBase(parent, pos) {}

			const KeyT operator*() {
				return this->mHashMap->GetKeyByIndex(this->mPos);
			}
		};

	public:
		Set(U32 size = DEFAULT_SIZE) :
			HashMap<KeyT, KeyT, Hasher>(size)
		{}

		KeyT* insert(KeyT key)
		{
			if (auto ret = this->find(key))
			{
				*ret = key;
				return ret;
			}

			if ((++this->mSize) >= this->mCapacityThreshold) {
				this->Grow();
			}
			return this->InsertImpl(this->HashKey(key), std::move(key), std::move(key));
		}

		Iterator begin() { return Iterator{ this, this->LookupIndex(0) }; }
		ConstIterator begin() const { return ConstIterator{ this, this->LookupIndex(0) }; }
		Iterator end() { return Iterator{ this, -1 }; }
		ConstIterator end() const { return ConstIterator{ this, -1 }; }
	};
}