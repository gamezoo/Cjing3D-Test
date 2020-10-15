#pragma once

#include "core\string\string.h"

namespace Cjing3D {
	namespace Logger
	{
		void Print(const char* format, ...);
		void Debug(const char* format, ...);
		void Info(const char* format, ...);
		void Warning(const char* format, ...);
		void Error(const char* format, ...);
		void Fatal(const char* format, ...);
	}

}