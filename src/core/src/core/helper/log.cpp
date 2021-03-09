#include "log.h"
#include "core\concurrency\concurrency.h"
#include "core\container\dynamicArray.h"
#include "core\container\staticArray.h"
#include "core\platform\platform.h"
#include "core\filesystem\file.h"
#include "core\helper\timer.h"

#include <stdarg.h>
#include <iostream>
#include <vector>

namespace Cjing3D
{
namespace Logger
{
	namespace
	{
		struct LogContext
		{
			static const int BUFFER_SIZE = 64 * 1024;
			StaticArray<char, BUFFER_SIZE> buffer_ = {};

			LogContext() {
				buffer_.fill('\0');
			}
		};
		thread_local LogContext mLogContext;

		struct LoggerImpl
		{
			Concurrency::Mutex mMutex;
			bool mDisplayTime = false;
			std::vector<LoggerSink*> mSinks; // DynamicArray<LoggerSink*>
		};
		static LoggerImpl mImpl;

		void LogImpl(LogLevel level, const char* msg, va_list args)
		{
			vsprintf_s(mLogContext.buffer_.data(), mLogContext.buffer_.size(), msg, args);
			{
				Concurrency::ScopedMutex lock(mImpl.mMutex);
				for (auto sink : mImpl.mSinks) {
					sink->Log(level, mLogContext.buffer_.data());
				}
			}
			mLogContext.buffer_.fill('\0');
		}
	}
	
	void SetIsDisplayTime(bool displayTime)
	{
		mImpl.mDisplayTime = displayTime;
	}

	bool IsDisplayTime()
	{
		return mImpl.mDisplayTime;
	}

	void RegisterSink(LoggerSink& sink)
	{
		Concurrency::ScopedMutex lock(mImpl.mMutex);
		for (auto ptr : mImpl.mSinks) 
		{
			if (ptr == &sink) {
				return;
			}
		}
		mImpl.mSinks.push_back(&sink);
	}

	void UnregisterSink(LoggerSink& sink)
	{
		Concurrency::ScopedMutex lock(mImpl.mMutex);
		for (auto it = mImpl.mSinks.begin(); it != mImpl.mSinks.end(); it++)
		{
			if ((*it) == &sink) 
			{
				mImpl.mSinks.erase(it);
				break;
			}
		}
	}

	void Log(LogLevel level, const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		LogImpl(level, msg, args);
		va_end(args);
	}

	void Print(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		LogImpl(LogLevel::LVL_DEV, msg, args);
		va_end(args);
	}

	void Info(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		LogImpl(LogLevel::LVL_INFO, msg, args);
		va_end(args);
	}

	void Warning(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		LogImpl(LogLevel::LVL_WARNING, msg, args);
		va_end(args);
	}

	void Error(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		LogImpl(LogLevel::LVL_ERROR, msg, args);
		va_end(args);
	}

	const char* GetPrefix(LogLevel level)
	{
		const char* prefix = "";
		switch (level)
		{
		case LogLevel::LVL_INFO:
			prefix = "[Info]";
			break;
		case LogLevel::LVL_WARNING:
			prefix = "[Warning]";
			break;
		case LogLevel::LVL_ERROR:
			prefix = "[Error]";
			break;
		}
		return prefix;
	}
}

void StdoutLoggerSink::Log(LogLevel level, const char* msg)
{
	switch (level)
	{
	case LogLevel::LVL_DEV:
		Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_WHITE);
		break;
	case LogLevel::LVL_INFO:
		Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_GREEN);
		break;
	case LogLevel::LVL_WARNING:
		Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_YELLOW);
		break;
	case LogLevel::LVL_ERROR:
		Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_RED);
		break;
	}

	if (Logger::IsDisplayTime())
	{
		auto timeStr = Timer::GetSystemTimeString();
		std::cout << timeStr << " ";
	}

	std::cout << Logger::GetPrefix(level) << " ";
	std::cout << msg << std::endl;
	Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_WHITE);
}

void FileLoggerSink::Log(LogLevel level, const char* msg)
{
	if (!mLogFile || !mLogFile->IsValid()) {
		return;
	}

	// time
	if (Logger::IsDisplayTime())
	{
		auto timeStr = Timer::GetSystemTimeString() + " ";
		mLogFile->Write(timeStr.c_str(), timeStr.length());
	}
	// prefix
	auto prefix = Logger::GetPrefix(level);
	mLogFile->Write(prefix, StringLength(prefix));
	// msg
	mLogFile->Write(msg, StringLength(msg));
	mLogFile->Write("\n", 1);
}

}