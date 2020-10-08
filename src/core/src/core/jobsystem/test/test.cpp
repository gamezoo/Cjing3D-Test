
#include "jobsystem\jobsystemFiber.h"
#include "jobsystem\concurrency.h"

#include "math\maths.h"
#include "helper\timer.h"
#include "helper\logger.h"

#define CATCH_CONFIG_MAIN
#include "catch\catch.hpp"

using namespace Cjing3D;

namespace
{
    Concurrency::Mutex loggingMutex;
    static const I32 MAX_FIBER_COUNT = 128;
    static const I32 FIBER_STACK_SIZE = 16 * 1024;

    void CalculatePrimes(I64 max)
    {
        std::vector<I64> primesFound;
        I64 numToTest = 2;
        while (primesFound.size() < max)
        {
            bool isPrime = true;
            for (I64 i = 2; i < numToTest; ++i)
            {
                I64 div = numToTest / i;
                if ((div * i) == numToTest)
                {
                    isPrime = false;
                    break;
                }
            }
            if (isPrime)
                primesFound.push_back(numToTest);
            numToTest++;
        }
    }

    void JobTest(I32 jobCount, const char* name)
    {
        std::vector<I32> jobDatas(jobCount);
        for (I32 i = 0; i < jobCount; i++) {
            jobDatas[i] = i;
        }

        std::vector<JobSystemFiber::JobInfo> jobInfos;
        jobInfos.reserve(jobCount);
        for (I32 i = 0; i < jobCount; i++)
        {
            JobSystemFiber::JobInfo jobInfo;
            jobInfo.jobName = "TestFunc";
            jobInfo.userParam_ = i + 1;
            jobInfo.userData_ = jobDatas.data();
            jobInfo.jobFunc_ = [](I32 param, void* data)
            {
                CalculatePrimes(100);
            };

            jobInfos.push_back(jobInfo);
        }

        JobSystemFiber::Counter* counter = nullptr;
        F64 timeStart = Timer::GetAbsoluteTime();
        JobSystemFiber::RunJobs(jobInfos.data(), jobInfos.size(), &counter);
        F64 runDeltaTime = Timer::GetAbsoluteTime() - timeStart;
        JobSystemFiber::WaitForCounter(counter);
        F64 totalTime = Timer::GetAbsoluteTime() - timeStart;
        F64 waitDelatTime = totalTime - runDeltaTime;

        Concurrency::ScopedMutex lock(loggingMutex);
        Logger::Print("***************************************************************************");
        Logger::Print("\"%s\"", name);
        Logger::Print("\tRubJobs: %f ms (%f ms. avg)", runDeltaTime / 1000.0, runDeltaTime / 1000.0 / (double)jobCount);
        Logger::Print("\tWaitForCounter: %f ms (%f ms. avg)", waitDelatTime / 1000.0, waitDelatTime / 1000.0 / (double)jobCount);
        Logger::Print("\tTotal: %f ms (%f ms. avg)", totalTime / 1000.0, totalTime / 1000.0 / (double)jobCount);
        Logger::Print("***************************************************************************");
    }
}

TEST_CASE("jobsystem-worker-1-job-1", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1, "jobsystem-worker-1-job-1");
}

TEST_CASE("jobsystem-worker-4-job-1", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1, "jobsystem-worker-4-job-1");
}

TEST_CASE("jobsystem-worker-8-job-1", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1, "jobsystem-worker-8-job-1");
}

TEST_CASE("jobsystem-worker-1-job-100", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(100, "jobsystem-worker-1-job-100");
}

TEST_CASE("jobsystem-worker-4-job-100", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(100, "jobsystem-worker-4-job-100");
}

TEST_CASE("jobsystem-worker-8-job-100", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(100, "jobsystem-worker-8-job-100");
}


TEST_CASE("jobsystem-worker-1-job-1000", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1000, "jobsystem-worker-1-job-1000");
}

TEST_CASE("jobsystem-worker-4-job-1000", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1000, "jobsystem-worker-4-job-1000");
}

TEST_CASE("jobsystem-worker-8-job-1000", "[jobsystem]")
{
    JobSystemFiber::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1000, "jobsystem-worker-8-job-1000");
}
