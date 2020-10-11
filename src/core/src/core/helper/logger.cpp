#include "logger.h"
#include "core\jobsystem\concurrency.h"
#include "core\helper\timer.h"

#include <fstream>
#include <chrono>
#include <iostream>
#include <stdarg.h>
#include <array>

namespace Cjing3D {
	namespace Logger {
		using std::string;

		namespace {
			const string errorLogFileName = "error.txt";
			std::ofstream errorFile;

			const string generalLogFileName = "logger.txt";
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

			void PrintImpl(const string& msg, std::ostream& out = std::cout)
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

			string msg = "[Debug]  " + std::string(logContext->buffer_.data());
			PrintImpl(msg);
		}

		void Info(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			PrintImpl("[Info]  " + std::string(logContext->buffer_.data()));
			PrintImpl("[Info]  " + std::string(logContext->buffer_.data()), GetLoggerFile());
		}

		void Warning(const char* format, ...)
		{
			LogContext* logContext = GetLogContext();
			va_list args;
			va_start(args, format);
			vsprintf_s(logContext->buffer_.data(), logContext->buffer_.size(), format, args);
			va_end(args);

			string warningMsg = "[Warning]  " + std::string(logContext->buffer_.data());
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

			string warningMsg = "[Error]  " + std::string(logContext->buffer_.data());
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

			string warningMsg = "[Fatal]  " + std::string(logContext->buffer_.data());;
			PrintImpl(warningMsg);
			PrintImpl(warningMsg, GetErrorFile());
			PrintImpl(warningMsg, GetLoggerFile());
		}
	}
}