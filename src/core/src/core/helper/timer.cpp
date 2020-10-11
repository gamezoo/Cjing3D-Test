#include "timer.h"
#include "debug.h"
#include "core\platform\platform.h"

#include <chrono>

#ifndef CJING3D_PLATFORM_WIN32
std::chrono::time_point<std::chrono::high_resolution_clock> CounterStart;
std::atomic_flag initialized = ATOMIC_FLAG_INIT;
#endif

namespace Cjing3D {

#ifdef CJING3D_PLATFORM_WIN32
	F64 mFrequency = 0;
	U64 mCounterStart = 0;
#else
	std::chrono::time_point<std::chrono::high_resolution_clock> mCounterStart;
#endif

	Timer::Timer() :
		mTimeStamp(0),
		mDeltaTime(0),
		mTotalDeltaTime(0),
		mIsRunning(false)
	{
	}

	Timer::~Timer()
	{
	}

	void Timer::Start()
	{
		if (mIsRunning) {
			return;
		}

#ifdef CJING3D_PLATFORM_WIN32
		LARGE_INTEGER li;
		if (!QueryPerformanceFrequency(&li)) {
			Debug::Error("QueryPerformanceFrequency failed!");
		}
		mFrequency = double(li.QuadPart) / 1000.0;

		QueryPerformanceCounter(&li);
		mCounterStart = li.QuadPart;
#else
		mCounterStart = std::chrono::high_resolution_clock::now();
#endif
		mIsRunning = true;
	}

	void Timer::RecordDeltaTime() const
	{
		TimeStamp curTimeStamp = GetTotalTime();

		mDeltaTime = curTimeStamp - mTimeStamp;
		mTotalDeltaTime += mDeltaTime;
		mTimeStamp = curTimeStamp;
	}

	void Timer::Stop()
	{
		if (!mIsRunning)
			return;

		mIsRunning = false;
	}

	void Timer::Restart()
	{
		mIsRunning = false;
		Start();
	}

	TimeInterval Timer::GetDeltaTime() const
	{
		if (mIsRunning) {
			RecordDeltaTime();
		}

		return mDeltaTime;
	}

	TimeInterval Timer::GetTotalDeltaTime() const
	{
		if (mIsRunning)
			RecordDeltaTime();

		return mTotalDeltaTime;
	}

	TimeStamp Timer::GetRecordedTimeStamp() const
	{
		return mTimeStamp;
	}

	EngineTime Timer::GetTime() const
	{
		if (mIsRunning) {
			RecordDeltaTime();
		}

		return { mDeltaTime, mTotalDeltaTime };
	}

	std::string Timer::GetSystemTimeString()
	{
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm ptm;
		localtime_s(&ptm, &tt);
		char date[60] = { 0 };
		sprintf_s(date, "%02d:%02d:%02d ",
			(int)ptm.tm_hour, (int)ptm.tm_min, (int)ptm.tm_sec);
		return std::string(date);
	}

	TimeStamp Timer::GetAbsoluteTime()
	{
		TimeStamp ret = 0;
#ifdef CJING3D_PLATFORM_WIN32
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);

		LARGE_INTEGER fli;
		QueryPerformanceFrequency(&fli);
		mFrequency = double(fli.QuadPart) / 1000.0;

		ret = TimeStamp(li.QuadPart) / mFrequency;
#else
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_seconds = now;
		ret = TimeStamp(elapsed_seconds.count());
#endif
		return ret;
	}

	TimeStamp Timer::GetTotalTime() const
	{
#ifdef CJING3D_PLATFORM_WIN32
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return TimeStamp(li.QuadPart - mCounterStart) / mFrequency;
#else
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_seconds = now - mCounterStart;
		return TimeStamp(elapsed_seconds.count());
#endif
	}
}
















