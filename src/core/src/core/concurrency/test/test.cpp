#ifdef CJING_TEST_JOBSYSTEM

#include "core\concurrency\jobsystem.h"
#include "core\concurrency\concurrency.h"

#include "math\maths.h"
#include "core\helper\timer.h"
#include "core\helper\debug.h"

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

        std::vector<JobSystem::JobInfo> jobInfos;
        jobInfos.reserve(jobCount);
        for (I32 i = 0; i < jobCount; i++)
        {
            JobSystem::JobInfo jobInfo;
            jobInfo.jobName = "TestFunc";
            jobInfo.userParam_ = i + 1;
            jobInfo.userData_ = jobDatas.data();
            jobInfo.jobFunc_ = [](I32 param, void* data)
            {
                CalculatePrimes(100);
            };

            jobInfos.push_back(jobInfo);
        }

        JobSystem::JobHandle handle = JobSystem::INVALID_HANDLE;
        F64 timeStart = Timer::GetAbsoluteTime();
        JobSystem::RunJobs(jobInfos.data(), jobInfos.size(), &handle);
        F64 runDeltaTime = Timer::GetAbsoluteTime() - timeStart;
        JobSystem::Wait(&handle);
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


    void JobTest2(I32 jobCount, const char* name)
    {
        JobSystem::JobHandle handle = JobSystem::INVALID_HANDLE;
        F64 timeStart = Timer::GetAbsoluteTime();
        JobSystem::RunJob([&](I32 param, void* data) {
            
            for (I32 i = 0; i < jobCount; i++)
            {
                JobSystem::JobHandle localHandle = JobSystem::INVALID_HANDLE;
                JobSystem::RunJob([&](I32 param, void* data) {
                        CalculatePrimes(100);
                    }, 
                    nullptr, & localHandle);
                JobSystem::Wait(&localHandle);
            }

        }, nullptr, &handle);
  
        F64 runDeltaTime = Timer::GetAbsoluteTime() - timeStart;
        JobSystem::Wait(&handle);
        F64 totalTime = Timer::GetAbsoluteTime() - timeStart;
        F64 waitDelatTime = totalTime - runDeltaTime;

        Concurrency::ScopedMutex lock(loggingMutex);
        Logger::Print("***************************************************************************");
        Logger::Print("\"%s\"", name);
        Logger::Print("\tRubJobs: %f ms", runDeltaTime / 1000.0);
        Logger::Print("\tWaitForCounter: %f ms ", waitDelatTime / 1000.0);
        Logger::Print("\tTotal: %f ms", totalTime / 1000.0);
        Logger::Print("***************************************************************************");
    }
}

TEST_CASE("jobsystem-test2-worker-1-job-10", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest2(5000, "jobsystem-test2-worker-1-job-10");
}

TEST_CASE("jobsystem-test2-worker-1-job-10", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest2(10, "jobsystem-test2-worker-1-job-10");
}

TEST_CASE("jobsystem-test2-worker-1-job-50", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest2(50, "jobsystem-test2-worker-1-job-50");
}

TEST_CASE("jobsystem-test2-worker-4-job-10", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest2(10, "jobsystem-test2-worker-4-job-10");
}

TEST_CASE("jobsystem-test2-worker-4-job-50", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest2(50, "jobsystem-test2-worker-4-job-50");
}

TEST_CASE("jobsystem-worker-4-job-1", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1, "jobsystem-worker-4-job-1");
}

TEST_CASE("jobsystem-worker-8-job-1", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1, "jobsystem-worker-8-job-1");
}

TEST_CASE("jobsystem-worker-1-job-100", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(100, "jobsystem-worker-1-job-100");
}

TEST_CASE("jobsystem-worker-4-job-100", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(100, "jobsystem-worker-4-job-100");
}

TEST_CASE("jobsystem-worker-8-job-100", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(100, "jobsystem-worker-8-job-100");
}


TEST_CASE("jobsystem-worker-1-job-1000", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(1, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1000, "jobsystem-worker-1-job-1000");
}

TEST_CASE("jobsystem-worker-4-job-1000", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(4, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1000, "jobsystem-worker-4-job-1000");
}

TEST_CASE("jobsystem-worker-8-job-1000", "[jobsystem]")
{
    JobSystem::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);
    JobTest(1000, "jobsystem-worker-8-job-1000");
}


TEST_CASE("JobSystemPrecondition", "[JobSystem]")
{
    JobSystem::ScopedManager scoped(8, MAX_FIBER_COUNT, FIBER_STACK_SIZE);

    I32 testValue = 0;
    JobSystem::JobHandle handle = JobSystem::INVALID_HANDLE;
    JobSystem::RunJob([&](I32 param, void* data) {
        for (int i = 0; i < 100; i++)
        {
            testValue += 1;
            Concurrency::Sleep(0.001f);
            Logger::Info("Waiting!!!!");
        }
        testValue += 1;

        }, nullptr, &handle, "TestA");

    JobSystem::JobHandle handle2 = JobSystem::INVALID_HANDLE;
    for (int i = 0; i < 10; i++)
    {
        JobSystem::RunJobEx([&](I32 param, void* data) {
            if (testValue >= 100)
                testValue += 100000;

            }, nullptr, &handle2, handle, "TestB");
    }
    JobSystem::Wait(&handle2);

    Logger::Print("RESULT:%d", testValue);
}

#endif