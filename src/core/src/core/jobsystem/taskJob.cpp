#include "taskJob.h"
#include "concurrency.h"

namespace Cjing3D
{
namespace JobSystem
{
	TaskJob::TaskJob(const char* name)
	{
		mJobInfo.jobName = name;
		mJobInfo.userData_ = this;
		mJobInfo.jobFunc_ = [](I32 param, void* data) {
			TaskJob* job = reinterpret_cast<TaskJob*>(data);
			job->OnWork(param);

			I32 runningTimes = Concurrency::AtomicDecrement(&job->mRunningTimes);
			if (runningTimes == 0) {
				job->OnCompleted();
			}
		};
	}

	TaskJob::~TaskJob()
	{
	}

	void TaskJob::RunTask(I32 param, Priority priority, JobHandle* jobHandle)
	{
		JobSystem::JobInfo jobInfo = mJobInfo;
		jobInfo.userParam_ = param;
		jobInfo.jobPriority_ = priority;

		RunJobs(&jobInfo, 1, jobHandle);
	}
}
}