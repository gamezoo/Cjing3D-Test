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