#include "concurrency.h"
#include "core\platform\platform.h"
#include "core\helper\debug.h"
#include "core\memory\memory.h"

#ifdef CJING3D_PLATFORM_WIN32
#include <array>
#endif

namespace Cjing3D
{
namespace Concurrency
{
	//////////////////////////////////////////////////////////////////////////
	// system utils
	//////////////////////////////////////////////////////////////////////////
	void YieldCPU()
	{
		::YieldProcessor();
	}

	void Sleep(F32 seconds)
	{
		::Sleep((DWORD)(seconds * 1000));
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

#ifdef CJING3D_PLATFORM_WIN32
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
		return impl->entryPointFunc_(impl->userData_);
	}

	struct MutexImpl
	{
		CRITICAL_SECTION critSec_;
		HANDLE lockedThread_;
		volatile I32 lockedCount_ = 0;
	};

#endif // CJING3D_PLATFORM_WIN32 

	Thread::Thread(EntryPointFunc entryPointFunc, void* userData, I32 stackSize, std::string debugName)
	{
#ifdef CJING3D_PLATFORM_WIN32
		mImpl = CJING_NEW(ThreadImpl);
		mImpl->entryPointFunc_ = entryPointFunc;
		mImpl->userData_ = userData;
		mImpl->threadHandle_ = ::CreateThread(nullptr, stackSize, ThreadEntryPoint, mImpl, 0, &mImpl->threadID_);

#ifdef DEBUG
		mImpl->debugName_ = debugName;
#endif
#endif
	}

	Thread::Thread(Thread&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
	}

	Thread::~Thread()
	{
#ifdef CJING3D_PLATFORM_WIN32
		if (mImpl != nullptr) {
			Join();
		}
#endif
	}

	Thread& Thread::operator=(Thread&& rhs)
	{
		std::swap(mImpl, rhs.mImpl);
		return *this;
	}

	void Thread::SetAffinity(U64 mask)
	{
#ifdef CJING3D_PLATFORM_WIN32
		if (mImpl != nullptr) {
			// Set thread affinity to physical cores
			::SetThreadAffinityMask(mImpl->threadHandle_, mask);
		}
#endif
	}

	I32 Thread::Join()
	{
#ifdef CJING3D_PLATFORM_WIN32
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
#endif
	}

	bool Thread::IsValid() const
	{
#ifdef CJING3D_PLATFORM_WIN32
		return mImpl != nullptr;
#endif
	}

	Mutex::Mutex()
	{
		// mutex use default new
		mImpl = new MutexImpl();
		::InitializeCriticalSection(&mImpl->critSec_);
	}

	Mutex::~Mutex()
	{
		if (mImpl != nullptr)
		{
			::DeleteCriticalSection(&mImpl->critSec_);

			// mutex use default delete
			delete mImpl;
		}
	}
	void Mutex::Lock()
	{
		::EnterCriticalSection(&mImpl->critSec_);
		if (AtomicIncrement(&mImpl->lockedCount_) == 1) {
			mImpl->lockedThread_ = ::GetCurrentThread();
		}
	}

	bool Mutex::TryLock()
	{
		if (!::TryEnterCriticalSection(&mImpl->critSec_))
		{
			if (AtomicIncrement(&mImpl->lockedCount_) == 1) {
				mImpl->lockedThread_ = ::GetCurrentThread();
			}
			return true;
		}
		return false;
	}

	void Mutex::Unlock()
	{
		if (AtomicDecrement(&mImpl->lockedCount_) == 0) {
			mImpl->lockedThread_ = nullptr;
		}
		::LeaveCriticalSection(&mImpl->critSec_);
	}

#ifdef CJING3D_PLATFORM_WIN32
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
		std::string debugName_;
	};

	Semaphore::Semaphore(I32 initialCount, I32 maximumCount, const std::string& debugName)
	{
		mImpl = CJING_NEW(SemaphoreImpl);
		mImpl->handle_ = ::CreateSemaphore(nullptr, initialCount, maximumCount, nullptr);
		mImpl->debugName_ = debugName;
	}

	Semaphore::~Semaphore()
	{
		::CloseHandle(mImpl->handle_);
		CJING_SAFE_DELETE(mImpl);
	}

	bool Semaphore::Signal(I32 count)
	{
		return ::ReleaseSemaphore(mImpl->handle_, count, nullptr);
	}

	bool Semaphore::Wait(I32 timeout)
	{
		return (::WaitForSingleObject(mImpl->handle_, timeout) == WAIT_OBJECT_0);
	}

	struct RWLockImpl
	{
		SRWLOCK mSRWLock = SRWLOCK_INIT;
	};
	RWLock::RWLock()
	{
		// mutex use default new
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

#endif

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
}
}