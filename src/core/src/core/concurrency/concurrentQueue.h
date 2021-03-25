#pragma once

#include "concurrency.h"
#include "core\container\list.h"

namespace Cjing3D
{
#define DEFAULT_QUEUE_MAX_COUNT INT_MAX

	template<typename T>
	class ConcurrentQueue
	{
	private:
		size_t mMaxCount = 0;
		List<T> mQueue;
		Concurrency::Mutex mMutex;
		//Concurrency::Semaphore mSem;

		template<typename E>
		bool PushImpl(E&& e)
		{
			Concurrency::ScopedMutex lock(mMutex);
			if (mQueue.size() >= mMaxCount) {
				return false;
			}
			mQueue.push_back(e);
			return true;
		}

	public:
		ConcurrentQueue(size_t maxCount = DEFAULT_QUEUE_MAX_COUNT) :
			mMaxCount(maxCount)
		{}
		~ConcurrentQueue()
		{
			clear();
		}

		ConcurrentQueue(const ConcurrentQueue& rhs) = delete;
		void operator=(const ConcurrentQueue& rhs) = delete;

		bool push_back(const T& val)
		{
			return PushImpl(val);
		}

		bool push_back(T&& val)
		{
			return PushImpl(val);
		}

		bool pop_back()
		{
			Concurrency::ScopedMutex lock(mMutex);
			if (mQueue.empty()) {
				return false;
			}

			mQueue.pop_back();
			return true;
		}

		void clear()
		{
			Concurrency::ScopedMutex lock(mMutex);
			mQueue.clear();
		}

		size_t size()const
		{
			Concurrency::ScopedMutex lock(mMutex);
			return mQueue.size();
		}
		T* front() 
		{ 
			Concurrency::ScopedMutex lock(mMutex);
			return mQueue.front();
		}
		T* back() 
		{
			Concurrency::ScopedMutex lock(mMutex);
			return mQueue.back();
		}
		const T* front()const 
		{
			Concurrency::ScopedMutex lock(mMutex);
			return mQueue.front();
		}
		const T* back()const 
		{
			Concurrency::ScopedMutex lock(mMutex);
			return mQueue.back();
		}

	};
}