#pragma once

// ASIO BEGIN
#ifdef CJING3D_NETWORK_ASIO
#pragma warning(disable : 4996)

#ifdef CJING3D_PLATFORM_WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif

#define ASIO_STANDALONE
#include "asio\include\asio.hpp"
#include "asio\include\asio\ts\buffer.hpp"
#include "asio\include\asio\ts\internet.hpp"
#endif
// ASIO END

#include "core\common\common.h"
#include "core\helper\stream.h"
#include "core\concurrency\concurrency.h"
#include "core\string\stringUtils.h"
#include "core\concurrency\concurrentQueue.h"
#include "core\signal\listener.h"
#include "core\helper\crtp.h"


namespace Cjing3D
{
namespace Network
{
	static size_t constexpr DEFAULT_TCP_BUFFER_SIZE = 1536;

	enum class NetEvent : U8
	{
		CONNECT,
		DISCONNECT,
		START,
		STOP,
		RECEIVE,

		COUNT
	};

	enum class ErrorCode : U8
	{
		SUCCESS = 0,
		FAIL_SEND = 1,
		TIMEOUT = 2,
		SERVER_FAIL = 3,
		KILLING_THREADS = 4
	};

	enum class ConnectionStatus : U8
	{
		Undefined,
		Connecting,
		Connected,
		Closing,
		Closed
	};

	class NetMessage
	{
	private:
		MemoryStream mStream;
		ErrorCode mErrorCode;

	public:
	};
	using MessagePtr = SharedPtr<NetMessage>;

}
}