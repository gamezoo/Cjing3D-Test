#include "jobsystemFiber.h"
#include "container\mpmc_bounded_queue.h"
#include "helper\debug.h"
#include "helper\logger.h"
#include "helper\timer.h"

#include <string>
#include <vector>
#include <array>

namespace Cjing3D
{
// job system profile enable
#define JOB_SYSTEM_PROFILE_ENABLE
// job system logging level
#define JOB_SYSTEM_LOGGING_LEVEL (0)

namespace JobSystemFiber
{
	class WorkerThread;
	class JobFiber;

	// worker wait time
	static const I32 WORKER_SEMAPHORE_WAITTIME = 100;


	//////////////////////////////////////////////////////////////////////////
	// ManagerImpl
	//////////////////////////////////////////////////////////////////////////

	struct ManagerImpl
	{
		std::vector<WorkerThread*> mWorkerThreads;
		MPMCBoundedQueue<JobFiber*> mFreeFibers;
		I32 mFiberStackSize = 0;
		Concurrency::Semaphore mScheduleSem = Concurrency::Semaphore(0, 65536);
		bool mIsExting = false;

		volatile I32 mOutOfFibers = 0;
		volatile I32 mJobCount = 0;

		// job queue based on priority
		std::array<MPMCBoundedQueue<JobInfo>, (I32)Priority::MAX> mPendingJobs;
		std::array<MPMCBoundedQueue<JobFiber*>, (I32)Priority::MAX> mWaitingFibers;

		// debug infos
#ifdef DEBUG
		volatile I32 mNumPendingJobs   = 0;
		volatile I32 mNumFreeFibers    = 0;
		volatile I32 mNumWaitingFibers = 0;
#endif

		bool GetJobFiber(JobFiber** ouputFiber);
		void ReleaseFiber(JobFiber* fiber, bool complete);
	};
	ManagerImpl* gManagerImpl = nullptr;

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
			JobSystemFiber::JobFiber* jobFiber = reinterpret_cast<JobSystemFiber::JobFiber*>(userData);
			while (!jobFiber->mIsExiting || jobFiber->mWorkFiber != nullptr)
			{
				// do job
				JobInfo& jobInfo = jobFiber->mJobInfo;
				if (jobInfo.jobFunc_ != nullptr) 
				{
					jobInfo.jobFunc_(jobInfo.userParam_, jobInfo.userData_);
					jobInfo.jobFunc_ = nullptr;
				}

				// update counter
				I32 jobCount = Concurrency::AtomicDecrement(&jobFiber->mJobInfo.mCounter->value_);
				if (jobCount == 0)
				{
					if (jobFiber->mJobInfo.mFreeCounter) {
						SAFE_DELETE(jobFiber->mJobInfo.mCounter);
					}
				}

				Concurrency::AtomicDecrement(&jobFiber->mManager.mJobCount);

				if (jobFiber->mWorkFiber != nullptr) {
					jobFiber->mWorkFiber->SwitchTo();
				}
			}
		}

		void SwitchTo(WorkerThread* worker , Concurrency::Fiber* workFiber)
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

	// workThread是一个负责调度的线程，该线程会将自身转换为Fiber，同时从manager中
	// 获取可用的jobFiber
	class WorkerThread
	{
	public:
		WorkerThread(I32 index, ManagerImpl& manager) :
			mManager(manager),
			mIndex(index)
		{
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

		static int ThreadEntryPointCallback(void* userData)
		{
			WorkerThread* worker = reinterpret_cast<WorkerThread*>(userData);

			// convert current thread to fiber
			Concurrency::Fiber workerFiber(Concurrency::Fiber::THIS_THREAD, "Job worker fiber");

			JobSystemFiber::ManagerImpl& manager = worker->mManager;
			JobSystemFiber::JobFiber* jobFiber = nullptr;

			// 仅当manager.mIsExiting=true时，结束循环
			while (manager.GetJobFiber(&jobFiber))
			{
				if (!jobFiber)
				{
					// 如果没有任务，则等待信号量，或超时后再次从Manager中获取
					manager.mScheduleSem.Wait(WORKER_SEMAPHORE_WAITTIME);
				}
				else
				{
#ifdef JOB_SYSTEM_PROFILE_ENABLE
#endif
					Concurrency::AtomicExchange(&worker->mMoveToWaitingFlag, 0);
					jobFiber->SwitchTo(worker, &workerFiber);

#ifdef JOB_SYSTEM_PROFILE_ENABLE
#endif
					bool complete = Concurrency::AtomicExchange(&worker->mMoveToWaitingFlag, 0) != 0;
					complete |= jobFiber->mJobInfo.jobFunc_ == nullptr;
					manager.ReleaseFiber(jobFiber, complete);
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
	};

	bool ManagerImpl::GetJobFiber(JobFiber** ouputFiber)
	{
		JobFiber* fiber = nullptr;
		*ouputFiber = nullptr;

		// 从Priority::High到Priority::low，先从PendingJobs获取job
		// 如果存在Job： 则从freeFiber中取得fiber去执行
		// 如果不存在job，则尝试从waitingFiber中取得fiber继续执行

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

				*ouputFiber = fiber;
				return true;
			}
		}

		return !mIsExting;
	}

	void ManagerImpl::ReleaseFiber(JobFiber* fiber, bool complete)
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
			while (!mWaitingFibers[prio].Enqueue(fiber)) 
			{
#if JOB_SYSTEM_LOGGING_LEVEL >= 1
				Logger::Warning("Failed to enqueue waiting fiber");
#endif
				Concurrency::SwitchToThread();
			}
#ifdef DEBUG
			Concurrency::AtomicIncrement(&mNumWaitingFibers);
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void Initialize(I32 numThreads, I32 numFibers, I32 fiberStackSize)
	{
		if (IsInitialized()) {
			return;
		}

		gManagerImpl = new ManagerImpl();
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
			gManagerImpl->mWorkerThreads.emplace_back(new WorkerThread(i, *gManagerImpl));
		}

		// init fibers
		for (I32 i = 0; i < numFibers; i++) 
		{
			gManagerImpl->mFreeFibers.Enqueue(new JobFiber(*gManagerImpl));
#ifdef DEBUG
			Concurrency::AtomicIncrement(&gManagerImpl->mNumFreeFibers);
#endif
		}
	}

	void Uninitialize()
	{
		if (!IsInitialized()) {
			return;
		}

		gManagerImpl->mIsExting = true;
		Concurrency::Barrier();
		gManagerImpl->mScheduleSem.Signal(gManagerImpl->mWorkerThreads.size());

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
			SAFE_DELETE(fiber);
		}

		// clear all work threads
		for (auto* workerThread : gManagerImpl->mWorkerThreads)
		{
			workerThread->mExiting = true;
			workerThread->mThread.Join();
			SAFE_DELETE(workerThread);
		}
		gManagerImpl->mWorkerThreads.clear();

		SAFE_DELETE(gManagerImpl);
	}

	bool IsInitialized()
	{
		return gManagerImpl != nullptr;
	}

	void BeginProfile()
	{
#ifdef JOB_SYSTEM_PROFILE_ENABLE
		if (!IsInitialized()) {
			return;
		}

#endif;
	}

	void EndProfile()
	{
#ifdef JOB_SYSTEM_PROFILE_ENABLE
		if (!IsInitialized()) {
			return;
		}

#endif;
	}

	void RunJobs(JobInfo* jobInfos, I32 numJobs, Counter** counter)
	{
		if (!IsInitialized()) {
			return;
		}

		const bool jobShouldFreeCounter = (counter == nullptr);
		Counter* localCounter = new Counter();
		localCounter->value_ = numJobs;

		Concurrency::AtomicAdd(&gManagerImpl->mJobCount, numJobs);

#if JOB_SYSTEM_LOGGING_LEVEL >= 1
		F64 startTime = Timer::GetAbsoluteTime();
		const F64 LOG_TIME_THRESHOLD = 100.0f / 1000000.0;    // 100us.
		const F64 LOG_TIME_REPEAT    = 1000.0f / 1000.0;      // 1000ms.
		F64 nextLogTime = startTime + LOG_TIME_THRESHOLD;
#endif

		for (int i = 0; i < numJobs; i++)
		{
			JobInfo& jobInfo = jobInfos[i];
			jobInfo.mCounter = localCounter;
			jobInfo.mFreeCounter = jobShouldFreeCounter;

			auto& pendingJobs = gManagerImpl->mPendingJobs[(I32)jobInfo.jobPriority_];
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
			gManagerImpl->mScheduleSem.Signal(1);

#ifdef DEBUG
			Concurrency::AtomicIncrement(&gManagerImpl->mNumPendingJobs);
#endif
		}

		if (counter != nullptr) {
			*counter = localCounter;
		}
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

			if (value == 0 && freeCounter) {
				SAFE_DELETE(counter);
			}
		}
	}

	void RunJobs(JobContext& context)
	{
		RunJobs(context.jobInfos_.data(), context.jobInfos_.size(), &context.counter_);
	}

	void Wait(JobContext& context)
	{
		WaitForCounter(context.counter_, 0);
	}

	ScopedManager::ScopedManager(I32 numThreads, I32 numFibers, I32 fiberStackSize)
	{
		JobSystemFiber::Initialize(numThreads, numFibers, fiberStackSize);
	}

	ScopedManager::~ScopedManager()
	{
		JobSystemFiber::Uninitialize();
	}

	void JobContext::Execute(const JobFunc& job, void* jobData, Priority priority, const std::string& jobName)
	{
		JobSystemFiber::JobInfo jobInfo;
		jobInfo.jobName = jobName;
		jobInfo.userParam_ = jobInfos_.size() + 1;
		jobInfo.userData_ = jobData;
		jobInfo.jobFunc_ = job;

		jobInfos_.push_back(jobInfo);
	}

	void JobContext::Dispatch(I32 jobCount, I32 groupSize, const JobFunc& jobFunc, Priority priority, const std::string& jobName)
	{
		DBG_ASSERT(jobInfos_.empty());

		if (jobCount == 0 || groupSize == 0) {
			return;
		}

		const I32 groupCount = (jobCount + groupSize - 1) / groupSize;
		jobGroupArgs_.reserve(groupCount);

		for (I32 groupID = 0; groupID < groupCount; groupID++)
		{
			I32 groupJobOffset = groupID * groupSize;
			I32 groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

			JobSystemFiber::JobInfo jobInfo;
			jobInfo.jobName = jobName;
			jobInfo.userParam_ = groupID;
			jobInfo.userData_ = &jobGroupArgs_[groupID];
			jobInfo.jobFunc_ = [groupJobOffset, groupJobEnd, jobFunc](I32 param, void* data)
			{
				JobGroupArgs groupArg;
				groupArg.groupID_ = param;

				for (I32 i = groupJobOffset; i < groupJobEnd; i++)
				{
					groupArg.groupIndex_ = i - groupJobOffset;
					groupArg.isFirstJobInGroup_ = (i == groupJobOffset);
					groupArg.isLastJobInGroup_ = (i == groupJobEnd - 1);

					jobFunc(i, &groupArg);
				}
			};

			jobInfos_.push_back(jobInfo);
		}
	}
}
}