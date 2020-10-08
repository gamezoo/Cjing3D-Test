#pragma once

#include "concurrency.h"

namespace Cjing3D
{
	namespace JobSystemFiber
	{
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
			Counter* mCounter = nullptr;
			bool mFreeCounter = false;
		};

		struct JobContext
		{
			struct JobGroupArgs
			{
				U32 groupID_;		   
				U32 groupIndex_;	       
				bool isFirstJobInGroup_;
				bool isLastJobInGroup_;	
			};

			std::vector<JobGroupArgs> jobGroupArgs_;
			std::vector<JobInfo> jobInfos_;
			Counter* counter_ = nullptr;

			void Execute(const JobFunc& job, void* jobData = nullptr, Priority priority = Priority::NORMAL, const std::string& jobName = "");
			void Dispatch(I32 jobCount, I32 groupSize, const JobFunc& jobFunc, Priority priority = Priority::NORMAL, const std::string& jobName = "");
		};

		void Initialize(I32 numThreads, I32 numFibers, I32 fiberStackSize);
		void Uninitialize();
		bool IsInitialized();
		void BeginProfile();
		void EndProfile();
		void YieldCPU();
		void WaitAll();

		void RunJobs(JobInfo* jobInfos, I32 numJobs, Counter** counter = nullptr);
		void WaitForCounter(Counter* counter, I32 value = 0, bool freeCounter = true);

		void RunJobs(JobContext& context);
		void Wait(JobContext& context);

		class ScopedManager
		{
		public:
			ScopedManager(I32 numThreads, I32 numFibers, I32 fiberStackSize);
			~ScopedManager();
		};
	};
}