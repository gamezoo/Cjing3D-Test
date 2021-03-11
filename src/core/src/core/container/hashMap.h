#pragma once

#include "core\memory\memory.h"
#include "core\helper\debug.h"
#include "math\hash.h"

namespace Cjing3D
{
	template<typename KeyT, typename ValueT, typename Hasher = Hasher<KeyT>>
	class HashMap
	{
	public:
		static const U32 DEFAULT_SIZE = 16;
		static const U32 RESIZE_PERCENT = 75;

		// iterator
		using HashMapT = HashMap<KeyT, ValueT, Hasher>;
		using KeyValuePair = std::pair<const KeyT&, ValueT&>;
		using ConstKeyValuePair = std::pair<const KeyT&, const ValueT&>;

		class IteratorBase
		{
		public:
			IteratorBase() = default;
			IteratorBase(const HashMapT* hashMap, I32 pos) : mHashMap(hashMap), mPos(pos) {}

			bool operator!= (const IteratorBase& other)
			{
				return mHashMap != other.mHashMap || mPos != other.mPos;
			}

			bool operator== (const IteratorBase& other)
			{
				return mHashMap == other.mHashMap && mPos == other.mPos;
			}

			IteratorBase& operator++()
			{
				if (mHashMap != nullptr) {
					mPos = mHashMap->LookupIndex(mPos + 1);
				}
				return *this;
			}

		protected:
			const HashMapT* mHashMap = nullptr;
			I32 mPos = -1;
		};

		class Iterator : public IteratorBase
		{
		public:
			Iterator(const HashMapT* parent, I32 pos)
				: IteratorBase(parent, pos)
			{
			}

			KeyValuePair operator*() {
				return KeyValuePair{
					this->mHashMap->mKeys[this->mPos],
					this->mHashMap->mValues[this->mPos]
				};
			}
		};

		class ConstIterator : public IteratorBase
		{
		public:
			ConstIterator(const HashMapT* parent, I32 pos)
				: IteratorBase(parent, pos)
			{
			}

			ConstKeyValuePair operator*() {
				return ConstKeyValuePair{
					this->mHashMap->mKeys[this->mPos],
					this->mHashMap->mValues[this->mPos]
				};
			}
		};

	protected:
		ContainerAllocator mAllocator;
		Hasher  mHasher;
		KeyT*   mKeys	   = nullptr;
		ValueT* mValues    = nullptr;
		U32*    mHashTable = nullptr;
		U32     mMask      = 0;
		U32     mCapacity  = 0;
		U32     mCapacityThreshold = 0;
		U32     mSize	   = 0;

	public:
		HashMap(U32 size = DEFAULT_SIZE) :
			mCapacity(size)
		{
			Alloc();
		}

		HashMap(const HashMap& rhs)
		{
			Copy(rhs);
		}

		HashMap(HashMap&& rhs)
		{
			Swap(rhs);
		}

		~HashMap()
		{
			if (mCapacity > 0)
			{
				for (size_t i = 0; i < mCapacity; i++)
				{
					if (mHashTable[i] != 0) {
						DestructByIndex(i);
					}
				}

				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mKeys);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mValues);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mHashTable);
			}
		}

		HashMap& operator=(const HashMap& rhs)
		{
			Copy(rhs);
			return *this;
		}

		HashMap& operator=(HashMap&& rhs)
		{
			Swap(rhs);
			return *this;
		}

		ValueT* find(const KeyT& key)
		{
			I32 pos = GetIndexByKey(key);
			return pos != -1 ? &mValues[pos] : nullptr;
		}

		const ValueT* find(const KeyT& key)const
		{
			I32 pos = GetIndexByKey(key);
			return pos != -1 ? &mValues[pos] : nullptr;
		}

		ValueT* insert(KeyT key, const ValueT& value)
		{
			if (auto ret = find(key))
			{
				*ret = value;
				return ret;
			}

			if ((++mSize) >= mCapacityThreshold) {
				Grow();
			}
			return InsertImpl(HashKey(key), std::move(key), std::move(value));
		}


		ValueT* insert(KeyT key, ValueT&& value)
		{
			if (auto ret = find(key))
			{
				*ret = std::move(value);
				return ret;
			}

			if ((++mSize) >= mCapacityThreshold) {
				Grow();
			}
			return InsertImpl(HashKey(key), std::move(key), std::move(value));
		}

		bool erase(KeyT key)
		{
			I32 pos = GetIndexByKey(key);
			if (pos == -1) {
				return false;
			}

			DestructByIndex(pos);
			mSize--;
			return true;
		}


		void reserve(U32 capacity)
		{
			if (capacity > mCapacity)
			{
				// record old values
				auto oldKeys = mKeys;
				auto oldValues = mValues;
				auto oldHashTable = mHashTable;
				auto oldCapacity = mCapacity;

				// allocate
				mCapacity = capacity;
				mKeys = nullptr;
				mValues = nullptr;
				mHashTable = nullptr;
				Alloc();

				// 将oldkeys和oldValue重新插入
				for (U32 i = 0; i < oldCapacity; i++)
				{
					U32 hash = oldHashTable[i];
					if (hash != 0) {
						InsertImpl(hash, std::move(oldKeys[i]), std::move(oldValues[i]));
					}
				}

				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, oldKeys);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, oldValues);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, oldHashTable);
			}
		}

		U32 size()const { return mSize; }
		bool empty() const { return mSize == 0; }
		void clear()
		{
			for (size_t i = 0; i < mCapacity; i++)
			{
				if (mHashTable[i] != 0) {
					DestructByIndex(i);
				}
			}
			mSize = 0;
		}

		const ValueT& operator[](const KeyT& key) const 
		{
			auto ret = find(key);
			Debug::CheckAssertion(ret, "HashMap: the key doese not exist");
			return *ret;
		}

		ValueT& operator[](const KeyT& key)
		{
			auto ret = find(key);
			if (ret == nullptr) {
				ret = insert(key, ValueT());
			}
			return *ret;
		}

		KeyT& GetKeyByIndex(I32 i)
		{
			return mKeys[i];
		}

		const KeyT& GetKeyByIndex(I32 i)const
		{
			return mKeys[i];
		}

		Iterator begin() { return Iterator{ this, LookupIndex(0) }; }
		ConstIterator begin() const { return ConstIterator{ this, LookupIndex(0) }; }
		Iterator end() { return Iterator{ this, -1 }; }
		ConstIterator end() const { return ConstIterator{ this, -1 }; }

	protected:
		I32 GetIndexByKey(const KeyT& key)const
		{
			U32 hash = HashKey(key);
			U32 pos = GetPosByHash(hash);
			U32 dist = 0;
			while (true)
			{
				if (dist >= mCapacity) {
					return -1;
				}
				if (mHashTable[pos] == 0) {
					return -1;
				}
				else if (mHashTable[pos] == hash && mKeys[pos] == key) {
					return pos;
				}
				
				pos = (pos + 1) & mMask;
				dist++;
			}
			return 0;
		}

		U32 HashKey(const KeyT& key)const
		{
			U64 hash = mHasher((U64)0, key);
			hash &= 0x7fffffff;
			hash |= hash == 0;
			return (U32)hash;
		}

		U32 GetPosByHash(U32 hash)const
		{
			return hash & mMask;
		}

		// 获取hash的期望位置和curPos的距离
		U32 ProbeDistanceHash(U32 hash, U32 curPos)
		{
			return (curPos - GetPosByHash(hash) + mCapacity) & mMask;
		}

		ValueT* InsertImpl(U32 hash, KeyT&& key, const ValueT& value)
		{
			U32 pos = GetPosByHash(hash);
			U32 dist = 0;
			ValueT* ret = nullptr;
			while (true)
			{
				// 如果指定位置为空，则直接在该位置创建key和value
				if (mHashTable[pos] == 0)
				{
					new (&mKeys[pos])  KeyT(std::move(key));
					new (&mValues[pos]) ValueT(value);
					mHashTable[pos] = hash;

					if (ret == nullptr) {
						ret = &mValues[pos];
					}
					return ret;
				}
				// 如果位置已被占用（冲突），使用线性开放式寻址寻找下一个可用位置
				else
				{
					// 获取当前已存在的对象的期望hashPos和pos的距离，如果这个距离小于dist，
					// 交换已存在对象和elem,之后则寻找已存在对象的下一个可用位置.
					// 目的是使得所有elem的位置偏移尽可能小，减少冲突次数

					// TODO: Too many copy constructs
					ValueT currentValue = value;
					U32 curElemProbeDist = ProbeDistanceHash(mHashTable[pos], pos);
					if (curElemProbeDist < dist)
					{
						std::swap(hash, mHashTable[pos]);
						std::swap(key, mKeys[pos]);

						// std::swap(value, mValues[pos]
						ValueT temp = mValues[pos];
						mValues[pos] = currentValue;
						currentValue = temp;

						dist = curElemProbeDist;

						if (ret == nullptr) {
							ret = &mValues[pos];
						}
					}
					pos = (pos + 1) & mMask;
					dist++;
				}
			}
			return ret;
		}

		ValueT* InsertImpl(U32 hash, KeyT&& key, ValueT&& value)
		{
			U32 pos = GetPosByHash(hash);
			U32 dist = 0;
			ValueT* ret = nullptr;
			while (true)
			{
				// 如果指定位置为空，则直接在该位置创建key和value
				if (mHashTable[pos] == 0)
				{
					new (&mKeys[pos])  KeyT(std::move(key));
					new (&mValues[pos]) ValueT(std::move(value));
					mHashTable[pos] = hash;

					if (ret == nullptr) {
						ret = &mValues[pos];
					}
					return ret;
				}
				// 如果位置已被占用（冲突），则寻找下一个可用位置
				else
				{
					// 如果当前elem的hash位置和实际位置的距离，小于dist
					// 则将当前值和elem交换，继续去寻找elem的可用位置
					// 这样可使所有elem的偏移尽可能小，较少循环次数
					U32 curElemProbeDist = ProbeDistanceHash(mHashTable[pos], pos);
					if (curElemProbeDist < dist)
					{
						std::swap(hash,  mHashTable[pos]);
						std::swap(key,   mKeys[pos]);
						std::swap(value, mValues[pos]);
						dist = curElemProbeDist;

						if (ret == nullptr) {
							ret = &mValues[pos];
						}
					}
					pos = (pos + 1) & mMask;
					dist++;
				}	
			}
			return ret;
		}

		void Alloc()
		{
			Debug::CheckAssertion(!mKeys && !mValues && !mHashTable);

			mKeys	   = (KeyT*)CJING_ALLOCATOR_MALLOC_ALIGN(mAllocator, mCapacity * sizeof(KeyT), alignof(KeyT));
			mValues    = (ValueT*)CJING_ALLOCATOR_MALLOC_ALIGN(mAllocator, mCapacity * sizeof(ValueT), alignof(ValueT));
			mHashTable = (U32*)CJING_ALLOCATOR_MALLOC_ALIGN(mAllocator, mCapacity * sizeof(U32), alignof(U32));

			for (U32 i = 0; i < mCapacity; i++) {
				mHashTable[i] = 0;
			}

			mCapacityThreshold = mCapacity * RESIZE_PERCENT / 100;
			mMask = mCapacity - 1;
		}

		void Grow()
		{
			// record old values
			auto oldKeys = mKeys;
			auto oldValues = mValues;
			auto oldHashTable = mHashTable;
			auto oldCapacity = mCapacity;

			// allocate
			mCapacity *= 2;
			mKeys = nullptr;
			mValues = nullptr;
			mHashTable = nullptr;
			Alloc();

			// 将oldkeys和oldValue重新插入
			for (U32 i = 0; i < oldCapacity; i++)
			{
				U32 hash = oldHashTable[i];
				if (hash != 0) {
					InsertImpl(hash, std::move(oldKeys[i]), std::move(oldValues[i]));
				}
			}

			CJING_ALLOCATOR_FREE_ALIGN(mAllocator, oldKeys);
			CJING_ALLOCATOR_FREE_ALIGN(mAllocator, oldValues);
			CJING_ALLOCATOR_FREE_ALIGN(mAllocator, oldHashTable);
		}

		void Swap(HashMap& rhs)
		{
			if (this != &rhs)
			{
				std::swap(mKeys, rhs.mKeys);
				std::swap(mValues, rhs.mValues);
				std::swap(mHashTable, rhs.mHashTable);
				std::swap(mMask, rhs.mMask);
				std::swap(mCapacity, rhs.mCapacity);
				std::swap(mCapacityThreshold, rhs.mCapacityThreshold);
				std::swap(mSize, rhs.mSize);
			}
		}

		void Copy(const HashMap& rhs)
		{
			if (this != &rhs)
			{
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mKeys);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mValues);
				CJING_ALLOCATOR_FREE_ALIGN(mAllocator, mHashTable);

				mKeys = nullptr;
				mValues = nullptr;
				mHashTable = nullptr;
				mCapacity = rhs.mCapacity;

				for (U32 i = 0; i < rhs.mCapacity; i++)
				{
					U32 hash = rhs.mHashTable[i];
					if (hash != 0) {
						InsertImpl(hash, std::move(rhs.mKeys[i]), std::move(rhs.mValues[i]));
					}
				}
			}
		}

		void DestructByIndex(size_t index)
		{
			mKeys[index].~KeyT();
			mValues[index].~ValueT();
			mHashTable[index] = 0;
		}

		I32 LookupIndex(I32 i)const
		{
			while (i < mCapacity)
			{
				U32 hash = mHashTable[i];
				if (hash != 0) {
					return i;
				}
				i++;
			}
			return -1;
		}
	};
}