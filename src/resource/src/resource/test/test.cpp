#define CJING_TEST_RESOURCE
#ifdef CJING_TEST_RESOURCE

#include "resource\resourceManager.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"
#include "core\filesystem\filesystem.h"

#define CATCH_CONFIG_MAIN
#include "catch\catch.hpp"

using namespace Cjing3D;

TEST_CASE("resource", "[resource]")
{
	std::cout << "Hello World!" << std::endl;
}

#endif