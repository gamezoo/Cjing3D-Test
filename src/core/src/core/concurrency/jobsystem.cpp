#include "jobsystem.h"
#include "core\container\mpmc_bounded_queue.h"
#include "core\helper\debug.h"
#include "core\helper\timer.h"
#include "core\helper\profiler.h"
#include "core\memory\linearAllocator.h"
#include "core\container\dynamicArray.h"

#include <array>

#ifdef CJING3D_PLATFORM_WIN32

namespace Cjing3D
{
// job system profile enable
#define JOB_SYSTEM_PROFILE_ENABLE
// job system logging level
#define JOB_SYSTEM_LOGGING_LEVEL (0)

namespace JobSystem
{
	class WorkerThread;
	class JobFiber;

	// worker wait time
	static const I32 WORKER_SEMAPHORE_WAITTIME = 100;
	// free job signal count
	static const U32 HANDLE_ID_MASK = 0xffFF;
	static const U32 HANDLE_GENERATION_MASK = 0xffFF0000;
	static const I32 JOB_SIGNAL_COUNT = 512;

	struct Counter
	{
		volatile I32 value_ = 0;
		JobInfo mNextJob;
		JobHandle mSibling = INVALID_HANDLE;
		U32 mGeneration = 0;	// use for check
	};

	//////////////////////////////////////////////////////////////////////////
	// ManagerImpl
	//////////////////////////////////////////////////////////////////////////
	struct ManagerImpl
	{
		ManagerImpl()
		{
			mCounterPool.resize(JOB_SIGNAL_COUNT);
			mFreeHandleQueue.Reset(JOB_SIGNAL_COUNT);
			for (U32 i = 0; i < JOB_SIGNAL_COUNT; ++i) {
				mFreeHandleQueue.Enqueue(i);
			}
		}

		DynamicArray<WorkerThread*> mWorkerThreads;
		MPMCBoundedQueue<JobFiber*> mFreeFibers;
		I32 mFiberStackSize = 0;
		Concurrency::ConditionMutex mScheduleLock;
		bool mIsExting = false;

		volatile I32 mOutOfFibers = 0;
		volatile I32 mJobCount = 0;

		// job queue based on priority
		std::array<MPMCBoundedQueue<JobInfo>, (I32)Priority::MAX> mPendingJobs;
		std::array<MPMCBoundedQueue<JobFiber*>, (I32)Priority::MAX> mWaitingFibers;

		// job signals
		MPMCBoundedQueue<U32> mFreeHandleQueue;
		std::vector<Counter> mCounterPool;

		// debug infos
#ifdef DEBUG
		volatile I32 mNumPendingJobs   = 0;
		volatile I32 mNumFreeFibers    = 0;
		volatile I32 mNumWaitingFibers = 0;
#endif
		void PushJobInfo(JobInfo& jobInfo);
		JobHandle AllocateHandle();
		void ReleaseHandle(JobHandle jobHandle);
		bool IsHandleValid(JobHandle jobHandle);
		bool IsHandleZero(JobHandle jobHandle);

		bool GetJobFiber(WorkerThread* worker, JobFiber** ouputFiber);
		void ReleaseFiber(WorkerThread* worker, JobFiber* fiber, bool complete);
	};
	ManagerImpl* gManagerImpl = nullptr;

	//////////////////////////////////////////////////////////////////////////
	// JobFiber
	//////////////////////////////////////////////////////////////////////////
	class JobFiber
	{
	public:
		JobFiber(ManagerImpl& manager) :
			mManager(manager)
		{
			mFiber = Concurrency::Fiber(FiberEntryPointCallback, this, manager.mFiberStackSize, "JobWorkFiber");
		}

		static void FiberEntryPointCallback(void* userData)
		{
			JobSystem::JobFiber* jobFiber = reinterpret_cast<JobSystem::JobFiber*>(userData);
			while (!jobFiber->mIsExiting || jobFiber->mWorkFiber != nullptr)
			{
				// do job
				JobInfo& jobInfo = jobFiber->mJobInfo;
				if (jobInfo.jobFunc_ != nullptr) 
				{
					Profiler::BeginCPUBlock("Job");

					jobInfo.jobFunc_(jobInfo.userParam_, jobInfo.userData_);
					jobInfo.jobFunc_ = nullptr;

					Profiler::EndCPUBlock();
				}

				// update counter
				if (jobFiber->mJobInfo.mHandle != INVALID_HANDLE) {
					jobFiber->mManager.ReleaseHandle(jobFiber->mJobInfo.mHandle);
				}

				// update job count later
				Concurrency::AtomicDecrement(&jobFiber->mManager.mJobCount);

				if (jobFiber->mWorkFiber != nullptr) {
					jobFiber->mWorkFiber->SwitchTo();
				}
			}
		}

		void SwitchTo(WorkerThread* worker, Concurrency::Fiber* workFiber)
		{
			mWorkFiber = workFiber;
			mWorker = worker;
			mFiber.SwitchTo();
		}

		JobInfo& GetJobInfo()
		{
			return mJobInfo;
		}

		void SetJobInfo(const JobInfo& jobInfo)
		{
			mJobInfo = jobInfo;
		}

	public:
		ManagerImpl& mManager;
		Concurrency::Fiber mFiber;
		Concurrency::Fiber* mWorkFiber = nullptr;
		WorkerThread* mWorker = nullptr;
		JobInfo mJobInfo;
		bool mIsExiting = false;
	};

	//////////////////////////////////////////////////////////////////////////
	// WorkThread
	//////////////////////////////////////////////////////////////////////////
	// workThread是一个负责调度的线程，该线程会将自身转换为Fiber，同时从manager中
	// 获取可用的jobFiber
	class WorkerThread
	{
	public:
		WorkerThread(I32 index, ManagerImpl& manager) :
			mManager(manager),
			mIndex(index)
		{
			// show in profiler
			Profiler::ShowInProfiler(true);

			// init waiting fibers
			for (auto& waitingFibers : mWorkerWaitingFibers) {
				waitingFibers = MPMCBoundedQueue<JobFiber*>(128);
			}
			// init pending jobs
			for (auto& pendingJobs : mWorkerPendingJobs) {
				pendingJobs = MPMCBoundedQueue<JobInfo>(128);
			}

			std::string debugName = std::string("Job worker thread") + std::to_string(index);
			mThread = Concurrency::Thread(ThreadEntryPointCallback, this, Concurrency::Thread::DEFAULT_STACK_SIZE, debugName);

			I32 numCores = Concurrency::GetNumPhysicalCores();
			U64 affinityMask = 1ull << (index % numCores);
			mThread.SetAffinity(affinityMask);
		}

		~WorkerThread()
		{
			mThread.Join();
		}

		void Sleep(Concurrency::ConditionMutex& lock, I32 time = INFINITE)
		{
			Concurrency::ScopedConditionMutex locker(lock);
			mThread.Sleep(lock, time);
		}

		void Wakeup(Concurrency::ConditionMutex& lock)
		{
			Concurrency::ScopedConditionMutex locker(lock);
			mThread.Wakeup();
		}

		static int ThreadEntryPointCallback(void* userData)
		{
			WorkerThread* worker = reinterpret_cast<WorkerThread*>(userData);
			JobSystem::ManagerImpl& manager = worker->mManager;

			// convert current thread to fiber
			Concurrency::Fiber workerFiber(Concurrency::Fiber::THIS_THREAD, "Job worker fiber");
			JobSystem::JobFiber* jobFiber = nullptr;

			// 仅当manager.mIsExiting=true时，结束循环
			while (manager.GetJobFiber(worker, &jobFiber))
			{
				if (!jobFiber)
				{
					// 如果没有任务，则等待CV，或超时后再次从Manager中获取
					PROFILE_CPU_BLOCK("sleeping");
					Profiler::ColorBlock(Color4::Pink());
					worker->Sleep(manager.mScheduleLock, WORKER_SEMAPHORE_WAITTIME);
				}
				else
				{
#ifdef JOB_SYSTEM_PROFILE_ENABLE
#endif
					Concurrency::AtomicExchange(&worker->mMoveToWaitingFlag, 0);
					jobFiber->SwitchTo(worker, &workerFiber);

#ifdef JOB_SYSTEM_PROFILE_ENABLE
#endif
					bool complete = Concurrency::AtomicExchange(&worker->mMoveToWaitingFlag, 0) == 0;
					complete |= jobFiber->mJobInfo.jobFunc_ == nullptr;
					manager.ReleaseFiber(worker, jobFiber, complete);
				}
			}

			while (!worker->mExiting) {
				Concurrency::SwitchToThread();
			}

			return 0;
		}

	public:
		ManagerImpl& mManager;
		Concurrency::Thread mThread;
		volatile I32 mMoveToWaitingFlag = 0;
		I32 mIndex = -1;
		bool mExiting = false;

		// exclusive jobs, the supporting nums of jobs is smaller then common jobQueue
		std::array<MPMCBoundedQueue<JobInfo>, (I32)Priority::MAX> mWorkerPendingJobs;
		std::array<MPMCBoundedQueue<JobFiber*>, (I32)Priority::MAX> mWorkerWaitingFibers;
	};

	//////////////////////////////////////////////////////////////////////////
	// ManagerImpl
	//////////////////////////////////////////////////////////////////////////

	void ManagerImpl::PushJobInfo(JobInfo& jobInfo)
	{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
		F64 startTime = Timer::GetAbsoluteTime();
		const F64 LOG_TIME_THRESHOLD = 100.0f / 1000000.0;    // 100us.
		const F64 LOG_TIME_REPEAT = 1000.0f / 1000.0;      // 1000ms.
		F64 nextLogTime = startTime + LOG_TIME_THRESHOLD;
#endif

		// 如果指定了workerThread,则将job添加到worker专属队列中
		if (jobInfo.mWorkerIndex != USE_ANY_WORKER)
		{
			WorkerThread* workerThread = mWorkerThreads[jobInfo.mWorkerIndex % mWorkerThreads.size()];
			auto& pendingJobs = workerThread->mWorkerPendingJobs[(I32)jobInfo.jobPriority_];
			while (!pendingJobs.Enqueue(jobInfo))
			{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1

				// log when enqueue job failed
				F64 currentTime = Timer::GetAbsoluteTime();
				if ((currentTime - startTime) > LOG_TIME_THRESHOLD)
				{
					if (currentTime > nextLogTime)
					{
						Logger::Warning("Failed to enqueue job, waiting for it (Total time waiting: %f ms)", (currentTime - startTime) * 1000.0);
						nextLogTime = currentTime + LOG_TIME_REPEAT;
					}
				}
#endif
				YieldCPU();
			}
			workerThread->Wakeup(mScheduleLock);
		}
		else
		{
			auto& pendingJobs = mPendingJobs[(I32)jobInfo.jobPriority_];
			while (!pendingJobs.Enqueue(jobInfo))
			{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
				// log when enqueue job failed
				F64 currentTime = Timer::GetAbsoluteTime();
				if ((currentTime - startTime) > LOG_TIME_THRESHOLD)
				{
					if (currentTime > nextLogTime)
					{
						Logger::Warning("Failed to enqueue job, waiting for it (Total time waiting: %f ms)", (currentTime - startTime) * 1000.0);
						nextLogTime = currentTime + LOG_TIME_REPEAT;
					}
				}
#endif
				YieldCPU();
			}
			// 通知worker去执行job
			for (auto* workerThread : mWorkerThreads) {
				workerThread->Wakeup(mScheduleLock);
			}
		}

#ifdef DEBUG
		Concurrency::AtomicIncrement(&mNumPendingJobs);
#endif
	}

	JobHandle ManagerImpl::AllocateHandle()
	{
		U32 handle = INVALID_HANDLE;
		if (!mFreeHandleQueue.Dequeue(handle)){
			return handle;
		}

		Counter& counter = mCounterPool[handle & HANDLE_ID_MASK];
		counter.value_ = 0;
		counter.mNextJob.jobFunc_ = nullptr;
		counter.mSibling = INVALID_HANDLE;

		return (handle & HANDLE_ID_MASK) | counter.mGeneration;
	}

	void ManagerImpl::ReleaseHandle(JobHandle jobHandle)
	{
		if (jobHandle == INVALID_HANDLE) {
			return;
		}

		Counter& counter = mCounterPool[jobHandle & HANDLE_ID_MASK];
		I32 jobCount = Concurrency::AtomicDecrement(&counter.value_);
		if (jobCount > 0) {
			return;
		}

		// add next jobs, TODO: need to use implicit mutex
		while (jobHandle != INVALID_HANDLE)
		{
			Counter& currCounter = mCounterPool[jobHandle & HANDLE_ID_MASK];
			if (currCounter.mNextJob.jobFunc_) {
				PushJobInfo(currCounter.mNextJob);
			}

			currCounter.mGeneration = (((currCounter.mGeneration >> 16) + 1) & 0xffFF) << 16;
			while (!mFreeHandleQueue.Enqueue((jobHandle & HANDLE_ID_MASK) | currCounter.mGeneration))
			{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
				Logger::Warning("Failed to enqueue free conunter");
#endif
				Concurrency::SwitchToThread();
			}

			currCounter.mNextJob.jobFunc_ = nullptr;
			jobHandle = currCounter.mSibling;
		}
	}

	bool ManagerImpl::IsHandleValid(JobHandle jobHandle)
	{
		return jobHandle != INVALID_HANDLE;
	}

	bool ManagerImpl::IsHandleZero(JobHandle jobHandle)
	{
		if (jobHandle == INVALID_HANDLE) {
			return true;
		}

		const U32 id = jobHandle & HANDLE_ID_MASK;
		const U32 gen = jobHandle & HANDLE_GENERATION_MASK;

		Counter& counter = mCounterPool[id];
		return counter.value_ <= 0 || counter.mGeneration != gen;
	}

	bool ManagerImpl::GetJobFiber(WorkerThread* worker, JobFiber** ouputFiber)
	{
		JobFiber* fiber = nullptr;
		*ouputFiber = nullptr;

		// 从Priority::High到Priority::low，先从PendingJobs获取job
		// 如果存在Job： 则从freeFiber中取得fiber去执行
		// 如果不存在job，则尝试从waitingFiber中取得fiber继续执行

		/// //////////////////////////////////////////////////////////////////////////
		/// handle worker exclusive jobQueue
		if (worker != nullptr)
		{
			for (I32 priority = 0; priority < (I32)Priority::MAX; priority++)
			{
				JobInfo jobInfo;
				if (worker->mWorkerPendingJobs[priority].Dequeue(jobInfo))
				{
#ifdef DEBUG
					Concurrency::AtomicDecrement(&mNumPendingJobs);
#endif

#if JOB_SYSTEM_LOGGING_LEVEL >= 1
					F64 startTime = Timer::GetAbsoluteTime();
					const F64 LOG_TIME_THRESHOLD = 100.0f / 1000000.0;    // 100us.
					const F64 LOG_TIME_REPEAT = 1000.0f / 1000.0;      // 1000ms.
					F64 nextLogTime = startTime + LOG_TIME_THRESHOLD;
#endif
					// get free fiber
					I32 spinCount = 0;
					I32 spinCountMax = 100;
					while (!mFreeFibers.Dequeue(fiber))
					{
						spinCount++;
						if (spinCount > spinCountMax) {
							Concurrency::AtomicIncrement(&mOutOfFibers);
						}

#if JOB_SYSTEM_LOGGING_LEVEL >= 1
						// log when enqueue job failed
						F64 currentTime = Timer::GetAbsoluteTime();
						if ((currentTime - startTime) > LOG_TIME_THRESHOLD)
						{
							if (currentTime > nextLogTime)
							{
								Logger::Warning("Failed to get free job fiber, waiting for it (Total time waiting: %f ms)", (currentTime - startTime) * 1000.0);
								nextLogTime = currentTime + LOG_TIME_REPEAT;
							}
						}
#endif
						Concurrency::SwitchToThread();

						// 如果所有的线程都陷入此循环中，则需要强制结束
						if (spinCount > spinCountMax)
						{
							if ((Concurrency::AtomicDecrement(&mOutOfFibers) + 1) == mWorkerThreads.size())
							{
								static bool breakHere = true;
								if (breakHere)
								{
									DBG_BREAK;
									breakHere = false;
								}
							}
						}
					}

					if (fiber == nullptr) {
						return false;
					}
#ifdef DEBUG
					Concurrency::AtomicDecrement(&mNumFreeFibers);
#endif
					* ouputFiber = fiber;
					fiber->SetJobInfo(jobInfo);

#if JOB_SYSTEM_LOGGING_LEVEL >= 2
					Logger::Info("Woker pending job \"%s\" (%u) being scheduled.", fiber->mJobInfo.jobName.c_str(), fiber->mJobInfo.userParam_);
#endif
					return true;
				}

				// waiting job fiber being rescheduled
				if (worker->mWorkerWaitingFibers[priority].Dequeue(fiber))
				{
#if JOB_SYSTEM_LOGGING_LEVEL >= 2
					Logger::Info("Woker waiting job \"%s\" (%u) being rescheduled.", fiber->mJobInfo.jobName.c_str(), fiber->mJobInfo.userParam_);
#endif
#ifdef DEBUG
					Concurrency::AtomicDecrement(&mNumWaitingFibers);
#endif
					* ouputFiber = fiber;
					return true;
				}
			}
		}


		/// //////////////////////////////////////////////////////////////////////////
		/// handle common jobQueue
		for (I32 priority = 0; priority < (I32)Priority::MAX; priority++)
		{
			JobInfo jobInfo;
			if (mPendingJobs[priority].Dequeue(jobInfo))
			{
#ifdef DEBUG
				Concurrency::AtomicDecrement(&mNumPendingJobs);
#endif

#if JOB_SYSTEM_LOGGING_LEVEL >= 1
				F64 startTime = Timer::GetAbsoluteTime();
				const F64 LOG_TIME_THRESHOLD = 100.0f / 1000000.0;    // 100us.
				const F64 LOG_TIME_REPEAT = 1000.0f / 1000.0;      // 1000ms.
				F64 nextLogTime = startTime + LOG_TIME_THRESHOLD;
#endif
				// get free fiber
				I32 spinCount = 0;
				I32 spinCountMax = 100;
				while (!mFreeFibers.Dequeue(fiber))
				{
					spinCount++;
					if (spinCount > spinCountMax) {
						Concurrency::AtomicIncrement(&mOutOfFibers);
					}

#if JOB_SYSTEM_LOGGING_LEVEL >= 1
					// log when enqueue job failed
					F64 currentTime = Timer::GetAbsoluteTime();
					if ((currentTime - startTime) > LOG_TIME_THRESHOLD)
					{
						if (currentTime > nextLogTime)
						{
							Logger::Warning("Failed to get free job fiber, waiting for it (Total time waiting: %f ms)", (currentTime - startTime) * 1000.0);
							nextLogTime = currentTime + LOG_TIME_REPEAT;
						}
					}
#endif
					Concurrency::SwitchToThread();

					// 如果所有的线程都陷入此循环中，则需要强制结束
					if (spinCount > spinCountMax)
					{
						if ((Concurrency::AtomicDecrement(&mOutOfFibers) + 1) == mWorkerThreads.size())
						{
							static bool breakHere = true;
							if (breakHere)
							{
								DBG_BREAK;
								breakHere = false;
							}
						}
					}
				}

				if (fiber == nullptr) {
					return false;
				}

#ifdef DEBUG
				Concurrency::AtomicDecrement(&mNumFreeFibers);
#endif

				* ouputFiber = fiber;
				fiber->SetJobInfo(jobInfo);

#if JOB_SYSTEM_LOGGING_LEVEL >= 2
				Logger::Info("Pending job \"%s\" (%u) being scheduled.", fiber->mJobInfo.jobName.c_str(), fiber->mJobInfo.userParam_);
#endif
				return true;
			}

			// waiting job fiber being rescheduled
			if (mWaitingFibers[priority].Dequeue(fiber))
			{
#if JOB_SYSTEM_LOGGING_LEVEL >= 2
				Logger::Info("Waiting job \"%s\" (%u) being rescheduled.", fiber->mJobInfo.jobName.c_str(), fiber->mJobInfo.userParam_);
#endif
#ifdef DEBUG
				Concurrency::AtomicDecrement(&mNumWaitingFibers);
#endif

				*ouputFiber = fiber;
				return true;
			}
		}

		return !mIsExting;
	}

	void ManagerImpl::ReleaseFiber(WorkerThread* worker, JobFiber* fiber, bool complete)
	{
#if JOB_SYSTEM_LOGGING_LEVEL >= 2
		Logger::Info("Job %s.\"%s\" (%u).", complete ? "complete" : "waiting", fiber->mJobInfo.jobName.c_str(), fiber->mJobInfo.userParam_);
#endif
		// if job is complete, fiber being free otherwise fiber being waiting
		if (complete)
		{
			while (!mFreeFibers.Enqueue(fiber)) 
			{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
				Logger::Warning("Failed to enqueue free fiber");
#endif
				Concurrency::SwitchToThread();
			}
#ifdef DEBUG
			Concurrency::AtomicIncrement(&mNumFreeFibers);
#endif
		}
		else
		{
			const I32 prio = (I32)fiber->GetJobInfo().jobPriority_;
			if ((I32)fiber->GetJobInfo().mWorkerIndex != USE_ANY_WORKER)
			{
				while (!worker->mWorkerWaitingFibers[prio].Enqueue(fiber))
				{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
					Logger::Warning("Failed to enqueue worker waiting fiber");
#endif
					Concurrency::SwitchToThread();
				}
			}
			else
			{
				while (!mWaitingFibers[prio].Enqueue(fiber))
				{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
					Logger::Warning("Failed to enqueue waiting fiber");
#endif
					Concurrency::SwitchToThread();
				}
			}
#ifdef DEBUG
			Concurrency::AtomicIncrement(&mNumWaitingFibers);
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Manager
	//////////////////////////////////////////////////////////////////////////

	void Initialize(I32 numThreads, I32 numFibers, I32 fiberStackSize)
	{
		if (IsInitialized()) {
			return;
		}

		gManagerImpl = CJING_NEW(ManagerImpl);
		gManagerImpl->mWorkerThreads.reserve(numThreads);
		gManagerImpl->mFreeFibers.Reset(numFibers);
		gManagerImpl->mFiberStackSize = fiberStackSize;

		// init waiting fibers
		for (auto& waitingFibers : gManagerImpl->mWaitingFibers) {
			waitingFibers = MPMCBoundedQueue<JobFiber*>(numFibers);
		}
		// init pending jobs
		for (auto& pendingJobs : gManagerImpl->mPendingJobs) {
			pendingJobs = MPMCBoundedQueue<JobInfo>(numFibers);
		}

		// init workThread
		for (I32 i = 0; i < numThreads; i++) {
			gManagerImpl->mWorkerThreads.push(CJING_NEW(WorkerThread(i, *gManagerImpl)));
		}

		// init fibers
		for (I32 i = 0; i < numFibers; i++) 
		{
			gManagerImpl->mFreeFibers.Enqueue(CJING_NEW(JobFiber(*gManagerImpl)));
#ifdef DEBUG
			Concurrency::AtomicIncrement(&gManagerImpl->mNumFreeFibers);
#endif		
		}

		Logger::Info("Jobsystem initialized; numThreads:%d", numThreads, numFibers);
	}

	void Uninitialize()
	{
		if (!IsInitialized()) {
			return;
		}

		gManagerImpl->mIsExting = true;
		Concurrency::Barrier();
		for (auto* workerThread : gManagerImpl->mWorkerThreads) {
			workerThread->Wakeup(gManagerImpl->mScheduleLock);
		}
		// gManagerImpl->mScheduleSem.Signal(gManagerImpl->mWorkerThreads.size());

		Concurrency::Fiber exitFiber(Concurrency::Fiber::THIS_THREAD, "Job Manager Deletion Fiber");
		DBG_ASSERT(exitFiber.IsValid());

		// wait for all jobs finished
		while (gManagerImpl->mJobCount > 0) {
			Concurrency::SwitchToThread();
		}

		// clear waiting fibers
		JobFiber* fiber = nullptr;
		for (auto& waitingFibers : gManagerImpl->mWaitingFibers) {
			waitingFibers.Dequeue(fiber);
		}

		// clear all fibers
		fiber = nullptr;
		while (gManagerImpl->mFreeFibers.Dequeue(fiber))
		{
			fiber->mIsExiting = true;
			fiber->SwitchTo(nullptr, nullptr);
			CJING_SAFE_DELETE(fiber);
		}

		// clear all work threads
		for (auto* workerThread : gManagerImpl->mWorkerThreads)
		{
			workerThread->mExiting = true;
			workerThread->mThread.Join();
			CJING_SAFE_DELETE(workerThread);
		}
		gManagerImpl->mWorkerThreads.clear();

		CJING_SAFE_DELETE(gManagerImpl);

		Logger::Info("Jobsystem uninitialized");
	}

	bool IsInitialized()
	{
		return gManagerImpl != nullptr;
	} 

	void RunJob(JobInfo jobInfo, JobHandle* jobHandle)
	{
		if (!IsInitialized()) {
			return;
		}

		RunJobs(&jobInfo, 1, jobHandle);
	}

	void RunJob(const JobFunc& job, void* jobData, JobHandle* jobHandle, const std::string& jobName)
	{
		if (!IsInitialized()) {
			return;
		}

		JobSystem::JobInfo jobInfo;
		jobInfo.jobName = jobName;
		jobInfo.userParam_ = 0;
		jobInfo.userData_ = jobData;
		jobInfo.jobPriority_ = Priority::NORMAL;
		jobInfo.jobFunc_ = job;
		jobInfo.mWorkerIndex = USE_ANY_WORKER;

		RunJobs(&jobInfo, 1, jobHandle);
	}

	void RunJob(const JobFunc& job, void* jobData, JobHandle* jobHandle, Priority priority, I32 workerIndex, const std::string& jobName)
	{
		if (!IsInitialized()) {
			return;
		}

		JobSystem::JobInfo jobInfo;
		jobInfo.jobName = jobName;
		jobInfo.userParam_ = 0;
		jobInfo.userData_ = jobData;
		jobInfo.jobPriority_ = priority;
		jobInfo.jobFunc_ = job;
		jobInfo.mWorkerIndex = workerIndex;

		RunJobs(&jobInfo, 1, jobHandle);
	}

	void RunJobEx(const JobFunc& job, void* jobData, JobHandle* jobHandle, JobHandle preConditon, const std::string& jobName)
	{
		if (!IsInitialized()) {
			return;
		}

		JobSystem::JobInfo jobInfo;
		jobInfo.jobName = jobName;
		jobInfo.userParam_ = 0;
		jobInfo.userData_ = jobData;
		jobInfo.jobPriority_ = Priority::NORMAL;
		jobInfo.jobFunc_ = job;
		jobInfo.mWorkerIndex = USE_ANY_WORKER;
		jobInfo.mPreconditon = preConditon;

		RunJobs(&jobInfo, 1, jobHandle);
	}

	void RunJobs(I32 jobCount, I32 groupSize, const JobGroupFunc& jobFunc, size_t sharedMemSize, JobHandle* jobHandle, Priority priority, const std::string& jobName)
	{
		if (!IsInitialized()) {
			return;
		}

		if (jobCount == 0 || groupSize == 0) {
			return;
		}

		std::vector<JobInfo> jobInfos;
		const I32 groupCount = (jobCount + groupSize - 1) / groupSize;
		for (I32 groupID = 0; groupID < groupCount; groupID++)
		{
			I32 groupJobOffset = groupID * groupSize;
			I32 groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

			JobSystem::JobInfo jobInfo;
			jobInfo.jobName = jobName;
			jobInfo.userParam_ = groupID;
			jobInfo.userData_ = nullptr;
			jobInfo.jobPriority_ = priority;
			jobInfo.jobFunc_ = [groupJobOffset, groupJobEnd, jobFunc, sharedMemSize](I32 param, void* data)
			{
				JobGroupArgs groupArg;
				groupArg.groupID_ = param;

				void* sharedMemData = nullptr;
				if (sharedMemSize > 0) {
					sharedMemData = Memory::StackAlloca(sharedMemSize);
				}

				for (I32 i = groupJobOffset; i < groupJobEnd; i++)
				{
					groupArg.groupIndex_ = i - groupJobOffset;
					groupArg.isFirstJobInGroup_ = (i == groupJobOffset);
					groupArg.isLastJobInGroup_ = (i == groupJobEnd - 1);

					jobFunc(i, &groupArg, sharedMemData);
				}
			};

			jobInfos.push_back(jobInfo);
		}

		RunJobs(jobInfos.data(), jobInfos.size(), jobHandle);
	}

	void RunJobs(JobInfo* jobInfos, I32 numJobs, JobHandle* jobHandle)
	{
		if (!IsInitialized()) {
			return;
		}

		// allocate counter
		JobHandle localHandle = [&]() -> JobHandle 
		{
			// if jobHandle is invalid handle, allocate a new handle
			if (!jobHandle) {
				return gManagerImpl->AllocateHandle();
			}
			if (*jobHandle != INVALID_HANDLE && !gManagerImpl->IsHandleZero(*jobHandle))  {
				return *jobHandle;
			}
			return gManagerImpl->AllocateHandle();
		}();
		if (localHandle == INVALID_HANDLE) {
			return;
		}
		if (jobHandle != nullptr) {
			*jobHandle = localHandle;
		}

		// set job count of handle
		Concurrency::AtomicAdd(&gManagerImpl->mCounterPool[localHandle & HANDLE_ID_MASK].value_, numJobs);

		// set total job count
		Concurrency::AtomicAdd(&gManagerImpl->mJobCount, numJobs);

#if JOB_SYSTEM_LOGGING_LEVEL >= 1
		F64 startTime = Timer::GetAbsoluteTime();
		const F64 LOG_TIME_THRESHOLD = 100.0f / 1000000.0;    // 100us.
		const F64 LOG_TIME_REPEAT = 1000.0f / 1000.0;      // 1000ms.
		F64 nextLogTime = startTime + LOG_TIME_THRESHOLD;
#endif

		for (int i = 0; i < numJobs; i++)
		{
			JobInfo& jobInfo = jobInfos[i];
			jobInfo.mHandle = localHandle;

			if (jobInfo.mPreconditon == INVALID_HANDLE || 
				gManagerImpl->IsHandleZero(jobInfo.mPreconditon))
			{
				gManagerImpl->PushJobInfo(jobInfo);
			}
			else
			{
				Counter& counter = gManagerImpl->mCounterPool[jobInfo.mPreconditon & HANDLE_ID_MASK];
				if (counter.mNextJob.jobFunc_)
				{
					// 如果已经存在后续任务,则创建新的Counter，并以链表形式添加到
					// Preconditon的counter中
					JobHandle newHandle = gManagerImpl->AllocateHandle();
					Counter& newCounter = gManagerImpl->mCounterPool[newHandle & HANDLE_ID_MASK];
					newCounter.mNextJob = jobInfo;
					newCounter.mSibling = counter.mSibling;
					counter.mSibling = newHandle;
				}
				else
				{
					counter.mNextJob = jobInfo;
				}
			}
		}
	}

	void Wait(JobHandle* jobHandle, I32 value)
	{
		if (!IsInitialized()) {
			return;
		}

		if (!jobHandle || *jobHandle == INVALID_HANDLE) {
			return;
		}

		if (gManagerImpl->IsHandleZero(*jobHandle)) {
			return;
		}

		// 如果当前线程没有纤程化，则直接sleep等待
		auto currentFiber = Concurrency::Fiber::GetCurrentFiber();
		if (!currentFiber)
		{
			Counter& counter = gManagerImpl->mCounterPool[*jobHandle & HANDLE_ID_MASK];
			while (counter.value_ > value) {
				Concurrency::Sleep(0.001f);
			}
			return;
		}

		// fiber profile
		PROFILE_FILBER_SWITCH(*jobHandle);

		// 否则会yield当前纤程，Jobsystem可能会执行优先级更高的job
		Counter& counter = gManagerImpl->mCounterPool[*jobHandle & HANDLE_ID_MASK];
		while (counter.value_ > value) {
			YieldCPU();
		}
//
//		if (value == 0) {
//			while (!gManagerImpl->mFreeHandleQueue.Enqueue(*jobHandle & HANDLE_ID_MASK))
//			{
//#if JOB_SYSTEM_LOGGING_LEVEL >= 1
//				Logger::Warning("Failed to enqueue jobHandle");
//#endif
//				Concurrency::SwitchToThread();
//			}
//			*jobHandle = INVALID_HANDLE;
//		}
	}

	void YieldCPU()
	{
		if (!IsInitialized()) {
			return;
		}

		auto currentFiber = Concurrency::Fiber::GetCurrentFiber();
		if (currentFiber)
		{
			JobFiber* jobFiber = reinterpret_cast<JobFiber*>(currentFiber->GetUserData());
			assert(jobFiber->mWorkFiber);
			assert(jobFiber->mWorker);

#if JOB_SYSTEM_LOGGING_LEVEL >= 2
			Logger::Info("Yield job: \"%s\"", jobFiber->mJobInfo.jobName.c_str());
#endif
			// move current fiber to waiting queue
			Concurrency::AtomicExchange(&jobFiber->mWorker->mMoveToWaitingFlag, 1);
			jobFiber->mWorkFiber->SwitchTo();
		}
		else
		{
			Concurrency::SwitchToThread();
		}
	}

	void WaitAll()
	{
		if (!IsInitialized()) {
			return;
		}

		PROFILE_FILBER_SWITCH(INVALID_HANDLE);

		while (gManagerImpl->mJobCount > 0) {
			YieldCPU();
		}
	}

	void WaitForCounter(Counter* counter, I32 value, bool freeCounter)
	{
		if (!IsInitialized()) {
			return;
		}

		if (counter != nullptr)
		{
			while (counter->value_ > value) {
				YieldCPU();
			}

			if (value <= 0 && freeCounter) {
				CJING_SAFE_DELETE(counter);
			}
		}
	}

	ScopedManager::ScopedManager(I32 numThreads, I32 numFibers, I32 fiberStackSize)
	{
		JobSystem::Initialize(numThreads, numFibers, fiberStackSize);
	}

	ScopedManager::~ScopedManager()
	{
		JobSystem::Uninitialize();
	}
}
}

#endif