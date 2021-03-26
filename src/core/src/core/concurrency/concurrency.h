#pragma once

#include "core\common\definitions.h"

#include <functional>

#ifndef CJING3D_PLATFORM_WIN32
#include <thread>
#include <mutex>
#endif

#include <thread>

namespace Cjing3D
{
namespace Concurrency
{
#ifdef CJING3D_PLATFORM_WIN32
	using ThreadID = U32;
#endif
	ThreadID GetCurrentThreadID();

	// system utils
	void YieldCPU();
	void Sleep(F32 seconds);
	void Barrier();
	void SwitchToThread();

	I32 GetNumPhysicalCores();
	U64 GetPhysicalCoreAffinityMask(I32 core);

	// atomic operator
	I32 AtomicDecrement(volatile I32* pw);
	I32 AtomicIncrement(volatile I32* pw);
	I32 AtomicAdd(volatile I32* pw, volatile I32 val);
	I32 AtomicAddAcquire(volatile I32* pw, volatile I32 val);
	I32 AtomicAddRelease(volatile I32* pw, volatile I32 val);
	I32 AtomicSub(volatile I32* pw, volatile I32 val);
	I32 AtomicExchange(volatile I32* pw, I32 exchg);
	I32 AtomicCmpExchange(volatile I32* pw, I32 exchg, I32 comp);
	I32 AtomicCmpExchangeAcquire(volatile I32* pw, I32 exchg, I32 comp);
	I32 AtomicExchangeIfGreater(volatile I32* pw, volatile I32 val);

	I64 AtomicDecrement(volatile I64* pw);
	I64 AtomicIncrement(volatile I64* pw);
	I64 AtomicAdd(volatile I64* pw, volatile I64 val);
	I64 AtomicAddAcquire(volatile I64* pw, volatile I64 val);
	I64 AtomicAddRelease(volatile I64* pw, volatile I64 val);
	I64 AtomicSub(volatile I64* pw, volatile I64 val);
	I64 AtomicExchange(volatile I64* pw, I64 exchg);
	I64 AtomicCmpExchange(volatile I64* pw, I64 exchg, I64 comp);
	I64 AtomicCmpExchangeAcquire(volatile I64* pw, I64 exchg, I64 comp);
	I64 AtomicExchangeIfGreater(volatile I64* pw, volatile I64 val);

	class Thread
	{
	public:
		using EntryPointFunc = std::function<int(void*)>;
		static const I32 DEFAULT_STACK_SIZE = 16 * 1024;

		Thread() = default;
		Thread(EntryPointFunc entryPointFunc, void* userData, I32 stackSize = DEFAULT_STACK_SIZE, std::string debugName = "");
		Thread(Thread&& rhs);
		~Thread();

		Thread& operator=(Thread&& rhs);

		void SetAffinity(U64 mask);
		I32 Join();
		bool IsValid()const;

	private:
		Thread(const Thread& rhs) = delete;
		Thread& operator=(const Thread& rhs) = delete;

#ifdef CJING3D_PLATFORM_WIN32
		struct ThreadImpl* mImpl = nullptr;
#else
		std::thread mThread;
#endif
	};

	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		void Lock();
		bool TryLock();
		void Unlock();

	private:
		Mutex(const Mutex& rhs) = delete;
		Mutex& operator=(const Mutex& rhs) = delete;

#ifdef CJING3D_PLATFORM_WIN32
		struct MutexImpl* mImpl = nullptr;
#else
		std::mutex mMutex;
#endif
	};

	class ScopedMutex
	{
	public:
		ScopedMutex(Mutex& mutex) :
			mMutex(mutex)
		{
			mMutex.Lock();
		}

		~ScopedMutex()
		{
			mMutex.Unlock();
		}

	private:
		ScopedMutex(const Mutex& rhs) = delete;
		ScopedMutex(Mutex&& rhs) = delete;

		Mutex& mMutex;
	};

#ifdef CJING3D_PLATFORM_WIN32
	// fiber local storage
	class FLS
	{
	public:
		FLS();
		~FLS();

		bool Set(void* data);
		void* Get();
		bool IsValid()const;

	private:
		FLS(const FLS& rhs) = delete;
		FLS& operator=(const FLS& rhs) = delete;

		I32 mHandle = -1;
	};

	// fiber is only available in win32
	class Fiber
	{
	public:
		using EntryPointFunc = std::function<void(void*)>;
		static const I32 DEFAULT_STACK_SIZE = 16 * 1024;

		enum ThisThread
		{
			THIS_THREAD
		};

		Fiber();
		Fiber(EntryPointFunc entryPointFunc, void* userData, I32 stackSize = DEFAULT_STACK_SIZE, std::string debugName = "");
		Fiber(ThisThread, std::string debugName = "");
		~Fiber();

		Fiber(Fiber&& rhs);
		Fiber& operator=(Fiber&& rhs);

		void SwitchTo();
		void* GetUserData();
		bool IsValid()const;

		static Fiber* GetCurrentFiber();

	private:
		Fiber(const Fiber& fiber) = delete;
		Fiber& operator=(const Fiber& rhs) = delete;

		struct FiberImpl* mImpl = nullptr;
	};

	class Semaphore
	{
	public:
		Semaphore(I32 initialCount, I32 maximumCount, const std::string& debugName = "");
		~Semaphore();

		bool Signal(I32 count);
		bool Wait(I32 timeout = -1);

	private:
		Semaphore(const Semaphore&) = delete;

		struct SemaphoreImpl* mImpl = nullptr;
	};

#endif

	class SpinLock
	{
	public:
		SpinLock();
		~SpinLock();

		void Lock();
		bool TryLock();
		void Unlock();

	private:
		volatile I32 mLockedFlag = 0;
	};

	class ScopedSpinLock final
	{
	public:
		ScopedSpinLock(SpinLock& spinLock)
			: mSpinLock(spinLock)
		{
			mSpinLock.Lock();
		}

		~ScopedSpinLock() { mSpinLock.Unlock(); }

	private:
		ScopedSpinLock(const ScopedSpinLock&) = delete;
		ScopedSpinLock(ScopedSpinLock&&) = delete;

		SpinLock& mSpinLock;
	};

	class RWLock final
	{
	public:
		RWLock();
		~RWLock();

		void BeginRead()const;
		void EndRead()const;
		void BeginWrite();
		void EndWrite();

	private:
		RWLock(const RWLock&) = delete;

		struct RWLockImpl* mImpl = nullptr;
		mutable U8 mImplData[8];
	};

	class ScopedReadLock final
	{
	public:
		ScopedReadLock(RWLock& lock)
			: mRWLock(lock)
		{
			mRWLock.BeginRead();
		}

		~ScopedReadLock() { mRWLock.EndRead(); }

	private:
		ScopedReadLock(const ScopedSpinLock&) = delete;
		ScopedReadLock(ScopedSpinLock&&) = delete;

		RWLock& mRWLock;
	};

	class ScopedWriteLock final
	{
	public:
		ScopedWriteLock(RWLock& lock)
			: mRWLock(lock)
		{
			mRWLock.BeginWrite();
		}

		~ScopedWriteLock() { mRWLock.EndWrite(); }

	private:
		ScopedWriteLock(const ScopedSpinLock&) = delete;
		ScopedWriteLock(ScopedSpinLock&&) = delete;

		RWLock& mRWLock;
	};

	class AtomicFlag
	{
	public:
		AtomicFlag();
		~AtomicFlag();

		void Clear();
		bool TestAndSet();

	private:
		AtomicFlag(const AtomicFlag&) = delete;

		volatile I32 mLockedFlag;
	};
}
}