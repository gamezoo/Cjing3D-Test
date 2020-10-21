#include "logger.h"

#include <fstream>
#include <chrono>
#include <iostream>
#include <stdarg.h>

namespace Cjing3D {
namespace LuaTools
{
	namespace Logger {
		using std::string;

		namespace {
			const string errorLogFileName = "error.txt";
			std::ofstream errorFile;

			const string generalLogFileName = "logger.txt";
			std::ofstream loggerFile;

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

			std::string GetCurSystemTimeStr()
			{
				auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				tm ptm;
				localtime_s(&ptm, &tt);
				char date[60] = { 0 };
				sprintf_s(date, "%02d:%02d:%02d ",
					(int)ptm.tm_hour, (int)ptm.tm_min, (int)ptm.tm_sec);
				return std::string(date);
			}
		}

		void Logger::Print(const string& msg, std::ostream& out)
		{
			auto timeStr = GetCurSystemTimeStr();
			out << timeStr << "    " << msg << std::endl;
		}

		void Debug(const string& msg)
		{
			Print("Debug: " + msg);
		}

		void Logger::Info(const string& msg)
		{
			Print("[Info]  " + msg);
			Print("[Info]  " + msg, GetLoggerFile());
		}

		void InfoEx(const char* format, ...)
		{
			char msg[256] = {};
			va_list args;
			va_start(args, format);
			vsnprintf_s(msg, std::size(msg), format, args);
			va_end(args);

			Print("[Info]  " + std::string(msg));
			Print("[Info]  " + std::string(msg), GetLoggerFile());
		}

		void Logger::Warning(const string& msg)
		{
			string warningMsg = "[Warning]  " + msg;
			Print(warningMsg);
			Print(warningMsg, GetLoggerFile());
		}

		void Logger::Error(const string& msg)
		{
			string warningMsg = "[Error]  " + msg;
			Print(warningMsg);
			Print(warningMsg, GetErrorFile());
			Print(warningMsg, GetLoggerFile());
		}

		void Fatal(const string& msg)
		{
			string warningMsg = "[Fatal]  " + msg;
			Print(warningMsg);
			Print(warningMsg, GetErrorFile());
			Print(warningMsg, GetLoggerFile());
		}
	}
}
}