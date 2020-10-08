#include "debug.h"
#include "logger.h"

#include <stdarg.h>
#include <stdexcept>

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

	namespace Debug {
		namespace {
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

		void Warning(const std::string & warningMsg)
		{
			Logger::Warning(warningMsg.c_str());
		}

		void Error(const std::string & errorMsg)
		{
			Logger::Error(errorMsg.c_str());
			if (DieOnError)
				abort();
		}

		void CheckAssertion(bool asertion)
		{
			if (!asertion)
				std::abort();
		}

		void CheckAssertion(bool assertion, const std::string & errorMsg)
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

		void Die(const std::string & dieMsg)
		{
			Logger::Fatal(dieMsg.c_str());
			if (ShowMsgBox) {
				MessageBox("ERROR", dieMsg.c_str(), MessageBoxType::OK, MessageBoxIcon::ICON_ERROR);
			}

			if (AbortOnDie) {
				std::abort();
			}

			Exception exception(dieMsg.c_str());
			throw exception;
		}

	}
}