#pragma once

#include "core\common\definitions.h"
#include "core\string\string.h"
#include "core\helper\log.h"

#include <exception>
#include <sstream>

namespace Cjing3D {

// 是否将log信息写入到文件中
#define CJING_LOG_WITH_FILE

	class Exception : public std::exception
	{
	public:
		Exception();
		Exception(const char* format, ...);
		Exception(const char* format, va_list args);
		virtual ~Exception();

		virtual const char* what() const noexcept override;

	private:
		char mMsg[2048];
	};

	namespace Debug
	{
		void SetDieOnError(bool t);
		bool IsDieOnError();
		void SetPopBoxOnDie(bool t);
		void SetAbortOnDie(bool t);
		void SetDebugConsoleEnable(bool t);
		bool IsDebugConsoleEnable();

		void DebugOuput(const char* msg);
		void Die(const char* format, ...);

		void CheckAssertion(bool assertion);
		void CheckAssertion(bool assertion, const char* errorMsg);

		void ThrowIfFailed(bool result);
		void ThrowIfFailed(bool result, const char* format, ...);
		void ThrowInvalidArgument(const char* format, ...);

		enum class MessageBoxType
		{
			OK = 0,
			OK_CANCEL,
			YES_NO,
			YES_NO_CANCEL
		};

		enum class MessageBoxIcon
		{
			ICON_WARNING = 0,
			ICON_ERROR,
			ICON_QUESTION
		};

		enum class MessageBoxReturn
		{
			OK = 0,
			YES = 0,
			NO = 1,
			CANCEL = 2,
		};

		MessageBoxReturn MessageBox(const char* title, const char* message, MessageBoxType type = MessageBoxType::OK, MessageBoxIcon icon = MessageBoxIcon::ICON_WARNING);

		bool AssertInternal(const char* Message, const char* File, int Line, ...);
	}

#ifdef DEBUG

#if CJING3D_PLATFORM_WIN32
#define DBG_BREAK __debugbreak()
#else
#define DBG_BREAK
#endif

#define ERR_FAIL_COND(mCond)																		\
	if (mCond) {																					\
		std::ostringstream oss;																		\
		oss << __FUNCTION__; oss << __FILE__; oss << __LINE__;										\
		oss << "Condition \"" _STR(mCond) "\" is true. returned: " _STR(mRet);					    \
		Logger::Error(oss.str().c_str());															\
		Debug::Die(oss.str().c_str());																\
	} else                                                                                          \
		((void)0)

#define ERR_FAIL_COND_V(mCond, mRet)															    \
	if (mCond) {																					\
		std::ostringstream oss;																		\
		oss << __FUNCTION__; oss << __FILE__; oss << __LINE__;										\
		oss << "Condition \"" _STR(mCond) "\" is true. returned: " _STR(mRet);					    \
		Logger::Error(oss.str().c_str());															\
		Debug::Die(oss.str().c_str());																\
		return mRet;                                                                                \
	} else                                                                                          \
		((void)0)

#define DBG_ASSERT_MSG(Condition, Message, ...)                                                                        \
	if(!(Condition))                                                                                                   \
	{                                                                                                                  \
		if(Debug::AssertInternal(Message, __FILE__, __LINE__, __VA_ARGS__))                                             \
			DBG_BREAK;                                                                                                 \
	}

#define DBG_ASSERT(Condition)                                                                                          \
	if(!(Condition))                                                                                                   \
	{                                                                                                                  \
		if(Debug::AssertInternal(#Condition, __FILE__, __LINE__))                                                       \
			DBG_BREAK;                                                                                                 \
	}

}
#else
#define ERR_FAIL_COND(mCond)																		\
#define ERR_FAIL_COND_V(mCond, mRet)															    \
#define DBG_ASSERT_MSG(Condition, Message, ...)                                                                        \
#define DBG_ASSERT(Condition)                                                                                          \

#endif

