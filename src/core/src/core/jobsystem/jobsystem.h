#pragma once

#include "concurrency.h"

namespace Cjing3D
{
	namespace JobSystem
	{
		using JobHandle = U32;
		constexpr JobHandle INVALID_HANDLE = 0xFFFFFFFF;

		using JobFunc = std::function<void(I32, void*)>;

		enum class Priority
		{
			HIGH = 0,
			NORMAL,
			LOW,

			MAX
		};

		struct Counter
		{
			volatile I32 value_ = 0;
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
			bool mFreeHandle = false;

			// not use
			Counter* mCounter = nullptr;
			bool mFreeCounter = false;
		};

		struct JobGroupArgs
		{
			U32 groupID_;
			U32 groupIndex_;
			bool isFirstJobInGroup_;
			bool isLastJobInGroup_;
		};

		void Initialize(I32 numThreads, I32 numFibers, I32 fiberStackSize);
		void Uninitialize();
		bool IsInitialized();
		void BeginProfile();
		void EndProfile();
		void YieldCPU();

		void RunJob(const JobFunc& job, void* jobData = nullptr, JobHandle* jobHandle = nullptr, Priority priority = Priority::NORMAL, const std::string& jobName = "");
		void RunJobs(I32 jobCount, I32 groupSize, const JobFunc& jobFunc, void* jobData = nullptr, JobHandle* jobHandle = nullptr, Priority priority = Priority::NORMAL, const std::string& jobName = "");
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