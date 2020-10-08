#pragma once

#include "common\definitions.h"

namespace Cjing3D {

	using TimeStamp = F64;
	using TimeInterval = F64;

	struct EngineTime
	{
		TimeInterval deltaTime;
		TimeInterval totalDeltaTime;
	};

	class Timer
	{
	public:
		Timer();
		~Timer();

		void Start();
		void Stop();
		void Restart();
		void RecordDeltaTime()const;

		TimeInterval GetDeltaTime()const;
		TimeInterval GetTotalDeltaTime()const;
		TimeStamp GetRecordedTimeStamp()const;
		EngineTime GetTime()const;

		static TimeStamp GetAbsoluteTime();

	private:
		TimeStamp GetTotalTime()const;

		mutable TimeStamp mTimeStamp;
		mutable TimeInterval mDeltaTime;
		mutable TimeInterval mTotalDeltaTime;

		bool mIsRunning = false;
	};

}