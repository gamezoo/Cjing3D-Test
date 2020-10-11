#include "jobsystem.h"
#include "core\container\mpmc_bounded_queue.h"
#include "core\helper\debug.h"
#include "core\helper\logger.h"
#include "core\helper\timer.h"

#include <string>
#include <vector>
#include <array>

#ifndef CJING3D_PLATFORM_WIN32

#include <thread>
#include <condition_variable>

namespace Cjing3D
{
namespace JobSystem
{
	const I32 MAX_JOB_COUNT = 256;

	//////////////////////////////////////////////////////////////////////////
	// ManagerImpl
	//////////////////////////////////////////////////////////////////////////
	struct ManagerImpl
	{
		std::vector<std::thread> mThreadPools;
		MPMCBoundedQueue<JobInfo> mPendingJobs;
		std::condition_variable mWakeCondition;
		std::mutex mWakeMutex;
		bool mIsExiting = false;

		bool Work();
	};
	ManagerImpl* gManagerImpl = nullptr;

	bool ManagerImpl::Work()
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	void Initialize(I32 numThreads, I32 numFibers, I32 fiberStackSize);
	{
		if (IsInitialized()) {
			return;
		}

		I32 numCores = std::thread::hardware_concurrency();
		numThreads = std::min(numThreads, numCores - 1);

		gManagerImpl = CJING_NEW(ManagerImpl);
		gManagerImpl->mThreadPools.reserve(numThreads);
		gManagerImpl->mPendingJobs.Reset(MAX_JOB_COUNT);

		for (uint32_t threadID = 0; threadID < numThreads; ++threadID)
		{
			gManagerImpl->mThreadPools[threadID] = std::thread([]{

				while (true)
				{
					if (!gManagerImpl->Work())
					{
						std::unique_lock<std::mutex> lock(gManagerImpl->mWakeMutex);
						gManagerImpl->mWakeCondition.wait(lock);
					}
				}
			});
			gManagerImpl->mThreadPools[threadID].detach();
		}
	}

	void Uninitialize()
	{
		if (!IsInitialized()) {
			return;
		}

		CJING_SAFE_DELETE(gManagerImpl);
	}

	bool IsInitialized()
	{
		return gManagerImpl != nullptr;
	}

	void BeginProfile()
	{
	}

	void EndProfile()
	{
	}

	void YieldCPU()
	{
	}
}
}

#endif