
#ifdef CJING_TEST_STRING

#include "core\string\string.h"
#include "core\memory\memTracker.h"

#define CATCH_CONFIG_MAIN
#include "catch\catch.hpp"

using namespace Cjing3D;

namespace
{
	bool RequireFunc(const bool l, const bool r)
	{
		return l == r;
	}

	bool RequireFunc(const int l, const int r)
	{
		return l == r;
	}

	bool RequireFunc(const char l, const char r)
	{
		return l == r;
	}

	bool RequireFunc(const char* l, const char* r)
	{
		return strcmp(l, r) == 0;
	}
}


TEST_CASE("string declare", "[string]")
{
	String a1 = "hello world!";
	String a2 = '!';
	String a3(5, 'f');
	String a4(a1);
	const char* test = "Funny";
	String a5(Span(test, 5));
	String a6(a3, 2, 2);

	std::cout << a1 << std::endl;
	std::cout << a2 << std::endl;
	std::cout << a3 << std::endl;
	std::cout << a4 << std::endl;
	std::cout << a5 << std::endl;
	std::cout << a6 << std::endl;
}

TEST_CASE("string operator", "[string]")
{
	String a = "Hello";
	String b = "World";
	String c = a + b;
	String d = "!!!";
	c += d;
	String e = d + "Boom";
	String f = d + 'c';
	String bigTest1 = "Hello World! It's a funny day!!!!";
	String g = d + bigTest1;
	String h = g + bigTest1;

	REQUIRE(RequireFunc(c.c_str(), "HelloWorld!!!"));
	REQUIRE(RequireFunc(e.c_str(), "!!!Boom"));
	REQUIRE(RequireFunc(e[1], '!'));
	REQUIRE(RequireFunc(e[3], 'B'));
	REQUIRE(RequireFunc(d.c_str(), "!!!"));
	REQUIRE(RequireFunc(f.c_str(), "!!!c"));
	REQUIRE(RequireFunc(g.c_str(), "!!!Hello World! It's a funny day!!!!"));
	REQUIRE(RequireFunc(h.c_str(), "!!!Hello World! It's a funny day!!!!Hello World! It's a funny day!!!!"));
}

TEST_CASE("string size method", "[string]")
{
	String smallTest = "Hello";
 	String bigTest = "1234567890987654321";
	String smallTest1 = smallTest;
	smallTest1.resize(12);
	String bigTest1 = bigTest;
	bigTest1.resize(24);

	REQUIRE(RequireFunc(smallTest.size(), 5));
	REQUIRE(RequireFunc(bigTest.size(), 19));
	REQUIRE(RequireFunc(smallTest1.size(), 12));
	REQUIRE(RequireFunc(bigTest1.size(), 24));

	bigTest1.resize(3);
	REQUIRE(RequireFunc(bigTest1.size(), 3));

	smallTest1.resize(64);
	REQUIRE(RequireFunc(smallTest1.size(), 64));

}

TEST_CASE("string method", "[string]")
{
	String smallTest = "Hello";
	String bigTest = "1234567890987654321";

	for (const auto& c : bigTest) {
		std::cout << c;
	}
	std::cout << std::endl;
	
	bigTest.insert(1, smallTest);

	REQUIRE(RequireFunc(bigTest.size(), 24));
	REQUIRE(RequireFunc(bigTest.c_str(), "1Hello234567890987654321"));

	bigTest.earse(1);

	REQUIRE(RequireFunc(bigTest.size(), 23));
	REQUIRE(RequireFunc(bigTest.c_str(), "1ello234567890987654321"));

	String test2 = bigTest.substr(1, 4);
	REQUIRE(RequireFunc(test2.c_str(), "ello"));
	REQUIRE(RequireFunc(test2.back(), 'o'));
	REQUIRE(RequireFunc(bigTest.back(), '1'));

	bigTest.clear();
	REQUIRE(RequireFunc(bigTest.c_str(), ""));

	bigTest = "1234567890987654321";
	test2 = bigTest.substr(2);

	bigTest.replace(1, 3, "ABCDEFGHIJ");
	REQUIRE(RequireFunc(bigTest.c_str(), "1ABCDEFGHIJ567890987654321"));
	bigTest.replace(1, 10, "234");
	REQUIRE(RequireFunc(bigTest.c_str(), "1234567890987654321"));
	bigTest.replace(1, 3, "234");
	REQUIRE(RequireFunc(bigTest.c_str(), "1234567890987654321"));

	int pos = test2.find('5');
	while (pos != String::npos)
	{
		test2.replace(pos, 1, "A");
		pos = test2.find('5', pos + 1);
	}
	REQUIRE(RequireFunc(test2.c_str(), "34A678909876A4321"));

	int rpos = test2.find_last_of('A');
	REQUIRE(RequireFunc(rpos, 12));
}

TEST_CASE("string comparsion", "[string]")
{
	String smallTest = "Hello";
	String bigTest = "1234567890987654321";
	String a = "Hello";
	String b = "123";
	String c = "456";
	REQUIRE(RequireFunc(smallTest != bigTest, true));
	REQUIRE(RequireFunc(smallTest == a, true));
	REQUIRE(RequireFunc(bigTest == a, false));
	REQUIRE(RequireFunc(c > b, true));
}

#ifdef CJING_MEMORY_TRACKER
TEST_CASE("string memory", "[string]")
{
	auto usageBefore = MemoryTracker::Get().GetMemUsage();
	for (int i = 0; i < 1000; i++)
	{
		String aa = "aaa";
		String bb = "1ABCDEFGHIJ567890987654321";
		String cc = aa + bb;
	}
	auto usageAfter = MemoryTracker::Get().GetMemUsage();
	REQUIRE(RequireFunc((int)usageBefore, (int)usageAfter));
}
#endif

#endif