#pragma once

#include "core\common\definitions.h"
#include "core\string\string.h"

namespace Cjing3D {

	using TimeStamp = F64;
	using TimeInterval = F64;

	struct EngineTime
	{
		TimeInterval deltaTime;
		TimeInterval totalDeltaTime;

		F32 GetDeltaTime();
		F32 GetTotalTime();
	};

	class Timer
	{
	public:
		Timer();
		~Timer();

		static Timer& Instance();

		void Start();
		void Stop();
		void Restart();
		void RecordDeltaTime()const;

		TimeInterval GetDeltaTime()const;
		TimeInterval GetTotalDeltaTime()const;
		TimeStamp GetRecordedTimeStamp()const;
		EngineTime GetTime()const;

		static String GetSystemTimeString();
		static TimeStamp GetAbsoluteTime();

	private:
		TimeStamp GetTotalTime()const;

		mutable TimeStamp mTimeStamp;
		mutable TimeInterval mDeltaTime;
		mutable TimeInterval mTotalDeltaTime;

		bool mIsRunning = false;
	};

}