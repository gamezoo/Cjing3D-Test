#include "debug.h"
#include "core\jobsystem\concurrency.h"
#include "core\helper\timer.h"
#include "core\string\string.h"
#include "core\common\version.h"
#include "core\container\staticArray.h"
#include "core\platform\platform.h"

#include <stdarg.h>
#include <stdexcept>
#include <fstream>
#include <iostream>

namespace Cjing3D
{
	Exception::Exception() :
		std::exception(),
		mMsg{}
	{
	}

	Exception::Exception(const char * format, ...) :
		Exception()
	{
		va_list args;
		va_start(args, format);
		vsnprintf_s(mMsg, std::size(mMsg), format, args);
		va_end(args);

		Debug::Error(mMsg);
	}

	Exception::Exception(const char * format, va_list args) :
		Exception()
	{
		vsnprintf_s(mMsg, std::size(mMsg), format, args);

		Debug::Error(mMsg);
	}

	Exception::~Exception() = default;

	const char* Exception::what() const noexcept
	{
		return mMsg;
	}

	namespace Logger
	{
		namespace {
			const String32 generalLogFileName = "log.txt";
			std::ofstream loggerFile;

			struct LogContext
			{
				static const int BUFFER_SIZE = 64 * 1024;
				StaticArray<char, BUFFER_SIZE> buffer_ = {};
			};
			thread_local LogContext mLogContext;

			LogContext* GetLogContext() { return &mLogContext; }

			std::ofstream& GetLoggerFile()
			{
				if (!loggerFile.is_open())
					loggerFile.open(generalLogFileName);
				return loggerFile;
			}

			Concurrency::Mutex mPrintMutex;

			void PrintImpl(const char* msg, const char* prefix = nullptr, std::ostream& out = std::cout)
			{
				Concurrency::ScopedMutex lock(mPrintMutex);
				auto timeStr = Timer::GetSystemTimeString();
				out << timeStr;
				if (prefix != nullptr) {
					out << " " << prefix;
				}
				out << " " << msg << std::endl;
			}
		}

		void Info(const char* msg, ...)
		{
			Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_GREEN);
			va_list args;
			va_start(args, msg);
			Logger::LogArgs(msg, args, "[Info]");
			va_end(args);
			Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_WHITE);
		}

		void LogArgs(const char* msg, va_list args, const char* prefix)
		{
			LogContext* logContext = GetLogContext();
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), msg, args);
			PrintImpl(logContext->buffer_.data(), prefix);

#ifdef CJING_LOG_WITH_FILE
			PrintImpl(logContext->buffer_.data(), prefix, GetLoggerFile());
#endif
		}

		void Log(const char* msg, const char* prefix, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, msg);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), msg, args);
			va_end(args);
			PrintImpl(logContext->buffer_.data(), prefix);

#ifdef CJING_LOG_WITH_FILE
			PrintImpl(logContext->buffer_.data(), prefix, GetLoggerFile());
#endif
		}

		void PrintConsoleHeader()
		{
			std::cout << "Cjing3D Version " << CjingVersion::GetVersionString() << std::endl;
			std::cout << "Copyright (c) 2019-2020 by ZZZY" << std::endl;
			std::cout << std::endl;
		}
	}

	namespace Debug {
		namespace {
			bool ShowDebugConsole = false;
			bool ShowMsgBox = false;
			bool AbortOnDie = false;
			bool DieOnError = false;
			bool debugWarningPause = true;
			bool enableBreakOnAssertion_ = true;
		}

		void SetDieOnError(bool t)
		{
			DieOnError = t;
		}

		void SetPopBoxOnDie(bool t)
		{
			ShowMsgBox = t;
		}

		void SetAbortOnDie(bool t)
		{
			AbortOnDie = t;
		}

		void SetDebugConsoleEnable(bool t)
		{
			ShowDebugConsole = t;
		}

		bool IsDebugConsoleEnable()
		{
			return ShowDebugConsole;
		}

		void Warning(const char* format, ...)
		{
			Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_YELLOW);
			va_list args;
			va_start(args, format);
			Logger::LogArgs(format, args, "[Warning]");
			va_end(args);
			Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_WHITE);
		}

		void Error(const char* format, ...)
		{
			Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_RED);
			va_list args;
			va_start(args, format);
			Logger::LogArgs(format, args, "[Error]");
			va_end(args);

			if (DieOnError) {
				abort();
			}
			Platform::SetLoggerConsoleFontColor(Platform::CONSOLE_FONT_WHITE);
		}

		void CheckAssertion(bool asertion)
		{
			if (!asertion)
				std::abort();
		}

		void CheckAssertion(bool assertion, const char* errorMsg)
		{
			if (!assertion) {
				Die(errorMsg);
			}
		}

		void ThrowIfFailed(bool result)
		{
			if (false == result) {
				throw Exception();
			}
		}

		void ThrowIfFailed(bool result, const char * format, ...)
		{
			if (false == result) {
				va_list args;
				va_start(args, format);
				Exception exception(format, args);
				va_end(args);

				throw exception;
			}
		}

		void ThrowInvalidArgument(const char* format, ...)
		{
			char msg[128];
			va_list args;
			va_start(args, format);
			vsnprintf_s(msg, std::size(msg), format, args);
			va_end(args);
			throw std::invalid_argument(msg);
		}

		MessageBoxReturn MessageBox(const char* title, const char* message, MessageBoxType type, MessageBoxIcon icon)
		{
			return MessageBoxReturn::OK;
		}

		bool AssertInternal(const char* Message, const char* File, int Line, ...)
		{
#if defined(DEBUG)
			char context[4096];
			va_list argList;
			va_start(argList, Line);
#if COMPILER_MSVC
			vsprintf_s(context, sizeof(context), Message, argList);
#else
			vsprintf(context, Message, argList);
#endif
			va_end(argList);

			Logger::Info("\"%s\" in %s on line %u.\n\nDo you wish to break?", context, File, Line);

			return enableBreakOnAssertion_;
#else
			return false;
#endif
		}

		void Die(const char* format, ...)
		{
			String128 buffer;
			va_list args;
			va_start(args, format);
			vsprintf_s(buffer.data(), buffer.size(), format, args);
			va_end(args);

			if (ShowMsgBox) {
				MessageBox("ERROR", buffer.c_str(), MessageBoxType::OK, MessageBoxIcon::ICON_ERROR);
			}

			if (AbortOnDie) {
				std::abort();
			}

			Exception exception(buffer.c_str());
			throw exception;
		}

	}
}