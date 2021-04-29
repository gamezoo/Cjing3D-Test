#pragma once

#include "concurrency.h"

namespace Cjing3D
{
	namespace JobSystem
	{
		static const I32 MAX_FIBER_COUNT = 256;
		static const I32 FIBER_STACK_SIZE = 6 * 1024;
		static const I32 USE_ANY_WORKER = 0xff;

		using JobHandle = U32;
		constexpr JobHandle INVALID_HANDLE = 0xFFFFFFFF;

		struct Counter;

		struct JobGroupArgs
		{
			U32 groupID_;
			U32 groupIndex_;
			bool isFirstJobInGroup_;
			bool isLastJobInGroup_;
		};

		using JobFunc = std::function<void(I32, void*)>;
		using JobGroupFunc = std::function<void(I32, JobGroupArgs*, void*)>;
		enum class Priority
		{
			HIGH = 0,
			NORMAL,
			LOW,

			MAX
		};

		// job description instance
		struct JobInfo
		{
			std::string jobName = "";
			JobFunc jobFunc_ = nullptr;
			Priority jobPriority_ = Priority::NORMAL;
			I32 userParam_ = 0;
			void* userData_ = nullptr;
			JobHandle mHandle = INVALID_HANDLE;
			I32 mWorkerIndex = USE_ANY_WORKER;
			JobHandle mPreconditon = INVALID_HANDLE;
		};

		void Initialize(I32 numThreads, I32 numFibers, I32 fiberStackSize);
		void Uninitialize();
		bool IsInitialized();
		void YieldCPU();

		void RunJob(JobInfo jobInfo, JobHandle* jobHandle = nullptr);
		void RunJob(const JobFunc& job, void* jobData, JobHandle* jobHandle, const std::string& jobName);
		void RunJob(const JobFunc& job, void* jobData = nullptr, JobHandle* jobHandle = nullptr, Priority priority = Priority::NORMAL, I32 workerIndex = USE_ANY_WORKER, const std::string& jobName = "");
		void RunJobEx(const JobFunc& job, void* jobData, JobHandle* jobHandle, JobHandle preConditon, const std::string& jobName);
		void RunJobs(I32 jobCount, I32 groupSize, const JobGroupFunc& jobFunc, size_t sharedMemSize = 0, JobHandle* jobHandle = nullptr,  Priority priority = Priority::NORMAL, const std::string& jobName = "");
		void RunJobs(JobInfo* jobInfos, I32 numJobs, JobHandle* jobHandle = nullptr);
		void Wait(JobHandle* jobHandle, I32 value = 0);
		void WaitAll();

		class ScopedManager
		{
		public:
			ScopedManager(I32 numThreads, I32 numFibers, I32 fiberStackSize);
			~ScopedManager();
		};
	};
}