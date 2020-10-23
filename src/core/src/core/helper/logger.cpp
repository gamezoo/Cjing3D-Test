#include "logger.h"
#include "core\jobsystem\concurrency.h"
#include "core\helper\timer.h"
#include "core\string\string.h"
#include "core\common\version.h"

#include <fstream>
#include <chrono>
#include <iostream>
#include <stdarg.h>
#include <array>

namespace Cjing3D {
	namespace Logger {

		namespace {
			const String32 errorLogFileName = "error.txt";
			std::ofstream errorFile;

			const String32 generalLogFileName = "logger.txt";
			std::ofstream loggerFile;

			struct LogContext
			{
				static const int BUFFER_SIZE = 64 * 1024;
				std::array<char, BUFFER_SIZE> buffer_ = {};
			};
			thread_local LogContext mLogContext;

			LogContext* GetLogContext() { return &mLogContext; }

			std::ofstream& GetErrorFile()
			{
				if (!errorFile.is_open())
					errorFile.open(errorLogFileName);
				return errorFile;
			}

			std::ofstream& GetLoggerFile()
			{
				if (!loggerFile.is_open())
					loggerFile.open(generalLogFileName);
				return loggerFile;
			}

			Concurrency::Mutex mPrintMutex;

			void PrintImpl(const String& msg, std::ostream& out = std::cout)
			{
				Concurrency::ScopedMutex lock(mPrintMutex);
				auto timeStr = Timer::GetSystemTimeString();
				out << timeStr << "    " << msg << std::endl;
			}
		}

		void Print(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);
			PrintImpl(logContext->buffer_.data());
		}

		void Debug(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			String msg = String("[Debug]  ") + String(logContext->buffer_.data());
			PrintImpl(msg);
		}

		void Info(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			PrintImpl(String("[Info]  ") + String(logContext->buffer_.data()));
			PrintImpl(String("[Info]  ") + String(logContext->buffer_.data()), GetLoggerFile());
		}

		void Warning(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			String warningMsg = String("[Warning]  ") + String(logContext->buffer_.data());
			PrintImpl(warningMsg);
			PrintImpl(warningMsg, GetLoggerFile());
		}

		void Logger::Error(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			String warningMsg = String("[Error]  ") + String(logContext->buffer_.data());
			PrintImpl(warningMsg);
			PrintImpl(warningMsg, GetErrorFile());
			PrintImpl(warningMsg, GetLoggerFile());
		}

		void Fatal(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			String warningMsg = String("[Fatal]  ") + String(logContext->buffer_.data());;
			PrintImpl(warningMsg);
			PrintImpl(warningMsg, GetErrorFile());
			PrintImpl(warningMsg, GetLoggerFile());
		}

		void PrintConsoleHeader()
		{
			std::cout << "Cjing3D Version " << CjingVersion::GetVersionString() << std::endl;
			std::cout << "Copyright (c) 2019-2020 by ZZZY" << std::endl;
			std::cout << std::endl;
		}
	}
}