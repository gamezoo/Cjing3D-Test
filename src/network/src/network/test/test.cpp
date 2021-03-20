#define CJING_NETWORK_TEST
#ifdef CJING_NETWORK_TEST

#pragma warning(disable : 4996)

#ifdef CJING3D_PLATFORM_WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif

#define ASIO_STANDALONE
#include "asio\include\asio.hpp"
#include "asio\include\asio\ts\buffer.hpp"
#include "asio\include\asio\ts\internet.hpp"

#include <iostream>

int main()
{
	std::cout << "Hello world!" << std::endl;
	return 0;
}

#endif