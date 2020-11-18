#pragma once

#include "jobsystem.h"

namespace Cjing3D
{
namespace JobSystem
{
	class TaskJob
	{
	public:
		TaskJob(const char* name);
		virtual ~TaskJob();

		void RunTaskImmediate(I32 param);
		void RunTask(I32 param, Priority priority = Priority::NORMAL, JobHandle* jobHandle = nullptr);

		virtual void OnWork(I32 param) = 0;
		virtual void OnCompleted() = 0;

	public:
		JobSystem::JobInfo mJobInfo;
		volatile I32 mRunningTimes = 0;
	};
}
}