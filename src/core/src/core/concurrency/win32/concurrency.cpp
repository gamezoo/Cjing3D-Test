#ifdef CJING3D_PLATFORM_WIN32

#include "core\concurrency\concurrency.h"
#include "core\platform\platform.h"
#include "core\helper\debug.h"
#include "core\helper\profiler.h"
#include "core\memory\memory.h"

#include <array>

namespace Cjing3D
{
namespace Concurrency
{
	ThreadID GetCurrentThreadID()
	{
		return ::GetCurrentThreadId();
	}

	//////////////////////////////////////////////////////////////////////////
	// system utils
	//////////////////////////////////////////////////////////////////////////
	void YieldCPU()
	{
		::YieldProcessor();
	}

	void Sleep(F32 millisecond)
	{
		::Sleep((DWORD)(millisecond * 1000));
	}

	void Barrier()
	{
		::MemoryBarrier();
	}

	void SwitchToThread()
	{
		::SwitchToThread();
	}

	I32 GetNumPhysicalCores()
	{
		I32 numCores = 0;
		std::array<SYSTEM_LOGICAL_PROCESSOR_INFORMATION, 256> info;
		I32 size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

		DWORD retLen = info.size() * size;
		::GetLogicalProcessorInformation(info.data(), &retLen);
		const I32 numInfos = retLen / size;	// calculate num of infos
		for (I32 index = 0; index < numInfos; index++)
		{
			if (info[index].Relationship == RelationProcessorCore) {
				numCores++;
			}
		}
		return numCores;
	}

	U64 GetPhysicalCoreAffinityMask(I32 core)
	{
		// get core affinity mask for thread::SetAffinity
		std::array<SYSTEM_LOGICAL_PROCESSOR_INFORMATION, 256> info;
		I32 size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

		DWORD retLen = info.size() * size;
		::GetLogicalProcessorInformation(info.data(), &retLen);
		const I32 numInfos = retLen / size;	// calculate num of infos
		I32 numCores = 0;
		for (I32 index = 0; index < numInfos; index++)
		{
			if (info[index].Relationship == RelationProcessorCore) 
			{
				if (numCores == core) {
					return info[index].ProcessorMask;
				}
				numCores++;
			}
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// I32
	//////////////////////////////////////////////////////////////////////////
	I32 AtomicDecrement(volatile I32* pw)
	{
		return InterlockedDecrement((LONG volatile*)pw);
	}

	I32 AtomicIncrement(volatile I32* pw)
	{
		return InterlockedIncrement((LONG volatile*)pw);
	}

	I32 AtomicAdd(volatile I32* pw, volatile I32 val)
	{
		return InterlockedAdd((LONG volatile*)pw, val);
	}

	I32 AtomicAddAcquire(volatile I32* pw, volatile I32 val)
	{
		return InterlockedAddAcquire((LONG volatile*)pw, val);
	}

	I32 AtomicAddRelease(volatile I32* pw, volatile I32 val)
	{
		return InterlockedAddRelease((LONG volatile*)pw, val);
	}

	I32 AtomicSub(volatile I32* pw, volatile I32 val)
	{
		return InterlockedExchangeAdd((LONG volatile*)pw, -(int32_t)val) - val;
	}

	I32 AtomicExchange(volatile I32* pw, I32 exchg)
	{
		return InterlockedExchange((LONG volatile*)pw, exchg);
	}

	I32 AtomicCmpExchange(volatile I32* pw, I32 exchg, I32 comp)
	{
		return InterlockedCompareExchange((LONG volatile*)(pw), exchg, comp);
	}

	I32 AtomicCmpExchangeAcquire(volatile I32* pw, I32 exchg, I32 comp)
	{
		return InterlockedCompareExchangeAcquire((LONG volatile*)(pw), exchg, comp);
	}

	I32 AtomicExchangeIfGreater(volatile I32* pw, volatile I32 val)
	{
		while (true)
		{
			I32 tmp = static_cast<I32 const volatile&>(*(pw));
			if (tmp >= val) {
				return tmp;
			}

			if (InterlockedCompareExchange((LONG volatile*)(pw), val, tmp) == tmp) {
				return val;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// I64
	//////////////////////////////////////////////////////////////////////////
	I64 AtomicDecrement(volatile I64* pw)
	{
		return InterlockedDecrement64((LONGLONG volatile*)pw);
	}

	I64 AtomicIncrement(volatile I64* pw)
	{
		return InterlockedIncrement64((LONGLONG volatile*)pw);
	}

	I64 AtomicAdd(volatile I64* pw, volatile I64 val)
	{
		return InterlockedAdd64((LONGLONG volatile*)pw, val);
	}

	I64 AtomicAddAcquire(volatile I64* pw, volatile I64 val)
	{
		return InterlockedAddAcquire64((LONGLONG volatile*)pw, val);
	}

	I64 AtomicAddRelease(volatile I64* pw, volatile I64 val)
	{
		return InterlockedAddRelease64((LONGLONG volatile*)pw, val);
	}

	I64 AtomicSub(volatile I64* pw, volatile I64 val)
	{
		return InterlockedExchangeAdd64((LONGLONG volatile*)pw, -(int64_t)val) - val;
	}

	I64 AtomicExchange(volatile I64* pw, I64 exchg)
	{
		return InterlockedExchange64((LONGLONG volatile*)pw, exchg);
	}

	I64 AtomicCmpExchange(volatile I64* pw, I64 exchg, I64 comp)
	{
		return InterlockedCompareExchange64((LONGLONG volatile*)pw, exchg, comp);
	}

	I64 AtomicCmpExchangeAcquire(volatile I64* pw, I64 exchg, I64 comp)
	{
		return InterlockedCompareExchangeAcquire64((LONGLONG volatile*)pw, exchg, comp);
	}

	I64 AtomicExchangeIfGreater(volatile I64* pw, volatile I64 val)
	{
		while (true)
		{
			I64 tmp = static_cast<I64 const volatile&>(*(pw));
			if (tmp >= val) {
				return tmp;
			}

			if (InterlockedCompareExchange64((LONGLONG volatile*)(pw), val, tmp) == tmp) {
				return val;
			}
		}
	}

	struct ThreadImpl
	{
		DWORD threadID_ = 0;
		HANDLE threadHandle_ = 0;
		Thread::EntryPointFunc entryPointFunc_ = nullptr;
		void* userData_ = nullptr;
		ConditionVariable mConditonVariable;
		std::string debugName_;
	};

	const DWORD MS_VC_EXCEPTION = 0x406D1388;

	static DWORD WINAPI ThreadEntryPoint(LPVOID lpThreadParameter)
	{
		ThreadImpl* impl = reinterpret_cast<ThreadImpl*>(lpThreadParameter);
		if (impl == nullptr) {
			return 0;
		}

#ifdef DEBUG
		// set thread debug name if in debug mode
		if (::IsDebuggerPresent() && !impl->debugName_.empty())
		{
#pragma pack(push, 8)
			typedef struct tagTHREADNAME_INFO
			{
				DWORD dwType;     /* must be 0x1000 */
				LPCSTR szName;    /* pointer to name (in user addr space) */
				DWORD dwThreadID; /* thread ID (-1=caller thread) */
				DWORD dwFlags;    /* reserved for future use, must be zero */
			} THREADNAME_INFO;
#pragma pack(pop)
			THREADNAME_INFO info;
			memset(&info, 0, sizeof(info));
			info.dwType = 0x1000;
			info.szName = impl->debugName_.c_str();
			info.dwThreadID = (DWORD)-1;
			info.dwFlags = 0;

			::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG), (const ULONG_PTR*)&info);
		}
#endif
		Profiler::SetCurrentThreadName(impl->debugName_.c_str());
		return impl->entryPointFunc_(impl->userData_);
	}

	Thread::Thread(EntryPointFunc entryPointFunc, void* userData, I32 stackSize, std::string debugName)
	{
		mImpl = CJING_NEW(ThreadImpl);
		mImpl->entryPointFunc_ = entryPointFunc;
		mImpl->userData_ = userData;
		mImpl->threadHandle_ = ::CreateThread(nullptr, stackSize, ThreadEntryPoint, mImpl, 0, &mImpl->threadID_);

#ifdef DEBUG
		mImpl->debugName_ = debugName;
#endif
	}

	Thread::Thread(Thread&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
	}

	Thread::~Thread()
	{
		if (mImpl != nullptr) {
			Join();
		}
	}

	Thread& Thread::operator=(Thread&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
		return *this;
	}

	void Thread::SetAffinity(U64 mask)
	{
		if (mImpl != nullptr) {
			// Set thread affinity to physical cores
			::SetThreadAffinityMask(mImpl->threadHandle_, mask);
		}
	}

	I32 Thread::Join()
	{
		if (mImpl != nullptr)
		{
			::WaitForSingleObject(mImpl->threadHandle_, INFINITE);

			DWORD exitCode = 0;
			::GetExitCodeThread(mImpl->threadHandle_, &exitCode);
			::CloseHandle(mImpl->threadHandle_);
			CJING_SAFE_DELETE(mImpl);
			return exitCode;
		}
		return 0;
	}

	bool Thread::IsValid() const
	{
		return mImpl != nullptr;
	}

	void Thread::Sleep(ConditionMutex& lock, I32 timeout)
	{
		mImpl->mConditonVariable.Sleep(lock, timeout);
	}

	void Thread::Wakeup()
	{
		mImpl->mConditonVariable.Wakeup();
	}

	struct MutexImpl
	{
		CRITICAL_SECTION critSec_;
		HANDLE lockedThread_;
		volatile I32 lockedCount_ = 0;
	};

	MutexImpl* Mutex::Get()
	{
		return reinterpret_cast<MutexImpl*>(&mImplData[0]);
	}

	Mutex::Mutex()
	{
		static_assert(sizeof(MutexImpl) <= sizeof(mImplData), "mImplData too small for MutexImpl!");
		memset(mImplData, 0, sizeof(mImplData));
		new(mImplData) MutexImpl();
		::InitializeCriticalSection(&Get()->critSec_);
	}

	Mutex::~Mutex()
	{
		::DeleteCriticalSection(&Get()->critSec_);
		Get()->~MutexImpl();
	}

	Mutex::Mutex(Mutex&& rhs)
	{
		rhs.Lock();
		std::swap(mImplData, rhs.mImplData);
		Unlock();
	}

	void Mutex::operator=(Mutex&& rhs)
	{
		rhs.Lock();
		std::swap(mImplData, rhs.mImplData);
		Unlock();
	}

	void Mutex::Lock()
	{
		DBG_ASSERT(Get() != nullptr);
		::EnterCriticalSection(&Get()->critSec_);
		if (AtomicIncrement(&Get()->lockedCount_) == 1) {
			Get()->lockedThread_ = ::GetCurrentThread();
		}
	}

	bool Mutex::TryLock()
	{
		DBG_ASSERT(Get() != nullptr);
		if (!!::TryEnterCriticalSection(&Get()->critSec_))
		{
			if (AtomicIncrement(&Get()->lockedCount_) == 1) {
				Get()->lockedThread_ = ::GetCurrentThread();
			}
			return true;
		}
		return false;
	}

	void Mutex::Unlock()
	{
		DBG_ASSERT(Get() != nullptr);
		DBG_ASSERT(Get()->lockedThread_ == ::GetCurrentThread());
		if (AtomicDecrement(&Get()->lockedCount_) == 0) {
			Get()->lockedThread_ = nullptr;
		}
		::LeaveCriticalSection(&Get()->critSec_);
	}

	FLS::FLS()
	{
		mHandle = ::FlsAlloc(nullptr);
	}

	FLS::~FLS()
	{
		::FlsFree(mHandle);
	}

	bool FLS::Set(void* data)
	{
		return ::FlsSetValue(mHandle, data);
	}

	void* FLS::Get()
	{
		return ::FlsGetValue(mHandle);
	}

	bool FLS::IsValid() const
	{
		return false;
	}

	struct FiberImpl
	{
		Fiber::EntryPointFunc entryPointFunc_ = nullptr;
		void* userData_ = nullptr;
		std::string debugName_;
		LPVOID fiber_ = nullptr;
		LPVOID nextFiber_ = nullptr;
		Fiber* parent_ = nullptr;
	};

	static FLS currentFiber;

	static void FiberEntryPoint(LPVOID lpThreadParameter)
	{
		FiberImpl* impl = reinterpret_cast<FiberImpl*>(lpThreadParameter);
		if (impl == nullptr) {
			return;
		}
		currentFiber.Set(impl);

		impl->entryPointFunc_(impl->userData_);

		// ensure nextfiber exists
		DBG_ASSERT(impl->nextFiber_);
		::SwitchToFiber(impl->nextFiber_);
	}

	Fiber::Fiber()
	{
	}

	// create fiber by entryPointFunc
	Fiber::Fiber(EntryPointFunc entryPointFunc, void* userData, I32 stackSize, std::string debugName)
	{
		mImpl = CJING_NEW(FiberImpl);
		mImpl->entryPointFunc_ = entryPointFunc;
		mImpl->userData_ = userData;
		mImpl->parent_ = this;
		mImpl->fiber_ = ::CreateFiber(stackSize, FiberEntryPoint, mImpl);
		mImpl->debugName_ = debugName;

		if (mImpl == nullptr) {
			CJING_SAFE_DELETE(mImpl);
		}
	}

	// conver current thread to fiber
	Fiber::Fiber(ThisThread, std::string debugName)
	{
		mImpl = CJING_NEW(FiberImpl);
		mImpl->entryPointFunc_ = nullptr;
		mImpl->userData_ = nullptr;
		mImpl->parent_ = this;
		mImpl->fiber_ = ::ConvertThreadToFiber(mImpl);
		mImpl->debugName_ = debugName;

		if (mImpl == nullptr) {
			CJING_SAFE_DELETE(mImpl);
		}
	}

	Fiber::~Fiber()
	{
		if (mImpl != nullptr)
		{
			if (mImpl->entryPointFunc_)
			{
				::DeleteFiber(mImpl->fiber_);
			}
			else
			{
				::ConvertFiberToThread();
			}
			CJING_SAFE_DELETE(mImpl);
		}
	}

	Fiber::Fiber(Fiber&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
		mImpl->parent_ = this;
	}

	Fiber& Fiber::operator=(Fiber&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
		mImpl->parent_ = this;
		return *this;
	}

	void Fiber::SwitchTo()
	{
		DBG_ASSERT(mImpl != nullptr);
		DBG_ASSERT(mImpl->parent_ == this);
		DBG_ASSERT(::GetCurrentFiber() != nullptr);

		void* currentFiber = ::GetCurrentFiber();
		if (currentFiber != nullptr && mImpl != nullptr && currentFiber != mImpl->fiber_)
		{
			Profiler::BeforeFiberSwitch();

			void* nextFiber = mImpl->nextFiber_;
			mImpl->nextFiber_ = mImpl->entryPointFunc_ != nullptr ? currentFiber : nullptr;
			::SwitchToFiber(mImpl->fiber_);
			mImpl->nextFiber_ = nextFiber;
		}
	}

	void* Fiber::GetUserData()
	{
		DBG_ASSERT(mImpl != nullptr);
		DBG_ASSERT(mImpl->parent_ == this);

		return mImpl->userData_;
	}

	bool Fiber::IsValid() const
	{
		return mImpl != nullptr;
	}

	Fiber* Fiber::GetCurrentFiber()
	{
		FiberImpl* impl = (FiberImpl*)currentFiber.Get();
		if (impl != nullptr) {
			return impl->parent_;
		}
		return nullptr;
	}

	struct SemaphoreImpl
	{
		HANDLE handle_;
	};

	SemaphoreImpl* Semaphore::Get()
	{
		return reinterpret_cast<SemaphoreImpl*>(&mImplData[0]);
	}

	Semaphore::Semaphore(I32 initialCount, I32 maximumCount, const char* debugName)
	{
		memset(mImplData, 0, sizeof(mImplData));
		new(mImplData) SemaphoreImpl();
		Get()->handle_ = ::CreateSemaphore(nullptr, initialCount, maximumCount, nullptr);
		mDebugName = debugName;
	}

	Semaphore::Semaphore(Semaphore&& rhs)
	{
		std::swap(mImplData, rhs.mImplData);
		std::swap(mDebugName, rhs.mDebugName);
	}

	Semaphore::~Semaphore()
	{
		::CloseHandle(Get()->handle_);
		Get()->~SemaphoreImpl();
	}

	bool Semaphore::Signal(I32 count)
	{
		DBG_ASSERT(Get() != nullptr);
		return ::ReleaseSemaphore(Get()->handle_, count, nullptr);
	}

	bool Semaphore::Wait(I32 timeout)
	{
		DBG_ASSERT(Get() != nullptr);
		return (::WaitForSingleObject(Get()->handle_, timeout) == WAIT_OBJECT_0);
	}

	struct RWLockImpl
	{
		SRWLOCK mSRWLock = SRWLOCK_INIT;
	};
	RWLock::RWLock()
	{
		// mutex use default new
		memset(mImplData, 0, sizeof(mImplData));
		mImpl = new(mImplData) RWLockImpl();
	}

	RWLock::~RWLock()
	{
#ifdef DEBUG
		if (!::TryAcquireSRWLockExclusive(&(mImpl->mSRWLock))) {
			DBG_ASSERT(false);
		}
#endif
	}

	void RWLock::BeginRead()const
	{
		::AcquireSRWLockShared(&mImpl->mSRWLock);
	}

	void RWLock::EndRead()const
	{
		::ReleaseSRWLockShared(&mImpl->mSRWLock);
	}

	void RWLock::BeginWrite()
	{
		::AcquireSRWLockExclusive(&mImpl->mSRWLock);
	}

	void RWLock::EndWrite()
	{
		::ReleaseSRWLockExclusive(&mImpl->mSRWLock);
	}

	SpinLock::SpinLock()
	{
	}

	SpinLock::~SpinLock()
	{
		Unlock();
	}

	void SpinLock::Lock()
	{
		while (AtomicCmpExchangeAcquire(&mLockedFlag, 1, 0) == 1) {
			YieldCPU();
		}
	}

	bool SpinLock::TryLock()
	{
		return (AtomicCmpExchangeAcquire(&mLockedFlag, 1, 0) == 0);
	}

	void SpinLock::Unlock()
	{
		AtomicExchange(&mLockedFlag, 0);
	}
	
	struct AtomicFlagImpl
	{
		volatile I32 mBooleanValue;
	};

	AtomicFlag::AtomicFlag() :
		mLockedFlag(0)
	{
	}

	AtomicFlag::~AtomicFlag()
	{
	}

	void AtomicFlag::Clear()
	{
		AtomicExchange(&mLockedFlag, 0);
	}

	bool AtomicFlag::TestAndSet()
	{
		return (AtomicCmpExchangeAcquire(&mLockedFlag, 1, 0) == 0);
	}

	ConditionVariable::ConditionVariable()
	{
		static_assert(sizeof(mImplData) >= sizeof(CONDITION_VARIABLE), "Size is not enough");
		static_assert(alignof(CONDITION_VARIABLE) == alignof(CONDITION_VARIABLE), "Alignment does not match");
		memset(mImplData, 0, sizeof(mImplData));
		CONDITION_VARIABLE* cv = new (mImplData) CONDITION_VARIABLE();
		::InitializeConditionVariable(cv);
	}

	ConditionVariable::~ConditionVariable()
	{
		((CONDITION_VARIABLE*)mImplData)->~CONDITION_VARIABLE();
	}

	void ConditionVariable::Sleep(ConditionMutex& lock, I32 timeout)
	{
		::SleepConditionVariableSRW((CONDITION_VARIABLE*)mImplData, (SRWLOCK*)lock.mImplData, timeout, 0);
	}

	ConditionVariable::ConditionVariable(ConditionVariable&& rhs)
	{
		std::swap(mImplData, rhs.mImplData);
	}

	void ConditionVariable::Wakeup()
	{
		::WakeConditionVariable((CONDITION_VARIABLE*)mImplData);
	}

	ConditionMutex::ConditionMutex()
	{
		static_assert(sizeof(mImplData) >= sizeof(SRWLOCK), "Size is not enough");
		static_assert(alignof(ConditionMutex) == alignof(SRWLOCK), "Alignment does not match");
		memset(mImplData, 0, sizeof(mImplData));
		SRWLOCK* lock = new (mImplData) SRWLOCK();
		InitializeSRWLock(lock);
	}

	ConditionMutex::~ConditionMutex()
	{
		SRWLOCK* lock = (SRWLOCK*)mImplData;
		lock->~SRWLOCK();
	}

	ConditionMutex::ConditionMutex(ConditionMutex&& rhs)
	{
		std::swap(mImplData, rhs.mImplData);
	}

	void ConditionMutex::Enter()
	{
		SRWLOCK* lock = (SRWLOCK*)mImplData;
		AcquireSRWLockExclusive(lock);
	}

	void ConditionMutex::Exit()
	{
		SRWLOCK* lock = (SRWLOCK*)mImplData;
		::ReleaseSRWLockExclusive(lock);
	}
}
}

#endif