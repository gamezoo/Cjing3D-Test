#pragma once

#include <string>
#include <ostream>
#include <iostream>

namespace Cjing3D {
namespace LuaTools
{
	namespace Logger
	{
		void Print(const std::string& msg, std::ostream& out = std::cout);
		void Debug(const std::string& msg);
		void Info(const std::string& msg);
		void InfoEx(const char* format, ...);
		void Warning(const std::string& msg);
		void Error(const std::string& msg);
		void Fatal(const std::string& msg);
	}
}
}