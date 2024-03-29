#include "debug.h"
#include "logger.h"

#include <stdarg.h>
#include <stdexcept>

namespace Cjing3D
{
namespace LuaTools
{
	Exception::Exception() :
		std::exception(),
		mMsg{}
	{
	}

	Exception::Exception(const char* format, ...) :
		Exception()
	{
		va_list args;
		va_start(args, format);
		vsnprintf_s(mMsg, std::size(mMsg), format, args);
		va_end(args);

		Debug::Error(mMsg);
	}

	Exception::Exception(const char* format, va_list args) :
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

	namespace Debug {
		namespace {
			bool AbortOnDie = false;
			bool DieOnError = false;
			bool debugWarningPause = true;
		}

		void SetDieOnError(bool t)
		{
			DieOnError = t;
		}

		void SetAbortOnDie(bool t)
		{
			AbortOnDie = t;
		}

		void Warning(const std::string& warningMsg)
		{
			Logger::Warning(warningMsg);
		}

		void Error(const std::string& errorMsg)
		{
			Logger::Error(errorMsg);
			if (DieOnError)
				abort();
		}

		void CheckAssertion(bool asertion)
		{
			if (!asertion)
				std::abort();
		}

		void CheckAssertion(bool assertion, const std::string& errorMsg)
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

		void ThrowIfFailed(bool result, const char* format, ...)
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

		void Die(const std::string& dieMsg)
		{
			Logger::Fatal(dieMsg);
			if (AbortOnDie) {
				std::abort();
			}

			Exception exception(dieMsg.c_str());
			throw exception;
		}

	}
}
}