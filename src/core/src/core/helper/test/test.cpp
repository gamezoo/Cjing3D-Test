
#ifdef CJING_TEST_REFLECTION

#include "core\helper\reflection.h"

#define CATCH_CONFIG_MAIN
#include "catch\catch.hpp"

using namespace Cjing3D;

class TestC
{
public:
	TestC() = default;

	int mTTT = 10;
};

class TestA : public TestC
{
public:
	TestA() = default;
	TestA(int value) : mValue(value) {
		std::cout << "TestA Constructor" << value << std::endl;
	}

	static void Destroy(TestA& testA)
	{
		std::cout << "Destroy!" << std::endl;
	}

	int mValue = 0;
	int mAge = 12;
	int mArray[10];

	static int mStaticValue;

	int FuncA(int b)
	{
		return b * 3;
	}
};

int TestA::mStaticValue = 10;


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


TEST_CASE("reflection", "simple test")
{
	Reflection::Reflect<TestC>(UID_HASH("TestC"))
		.AddVar<&TestC::mTTT>(UID_HASH("mTTT"));

	Reflection::Reflect<TestA>(UID_HASH("TestA"))
		.Base<TestC>()
		.AddCtor<int>()
		.AddDtor<&TestA::Destroy>()
		.AddVar<&TestA::mStaticValue>(UID_HASH("mStaticValue"))
		.AddFunc<&TestA::FuncA>(UID_HASH("FuncA"));

	TestA::mStaticValue = 12;

	TestA testA;

	Reflection::Any aaa(testA);
	Reflection::Any bbb(testA);
	if (aaa == bbb) {

	}

	//////////////////////////////////////////////////
	// variable
	auto typeInfo = Reflection::Resolve<TestA>();
	auto varInfo = typeInfo.Var(UID_HASH("mStaticValue"));
	if (varInfo)
	{
		auto value = varInfo.Get({});
		if (value) {
			std::cout << *value.TryCast<int>() << std::endl;
		}

		varInfo.Set({}, 4);
	}
	std::cout << TestA::mStaticValue << std::endl;

	//////////////////////////////////////////////////
	// function
	auto funcInfo = typeInfo.Func(UID_HASH("FuncA"));
	if (funcInfo)
	{
		auto ret = funcInfo.Invoke(testA, 4);
		if (ret)
		{
			std::cout << "FuncA ret: " << *ret.TryCast<int>() << std::endl;
		}
	}

	//////////////////////////////////////////////////
	// constructor
	auto ctor = typeInfo.Ctor<int>();
	if (ctor)
	{
		auto ret = ctor.Invoke(2);
		if (ret)
		{
			auto& testA = *ret.TryCast<TestA>();
			std::cout << "Ctor ret: " << testA.mValue << std::endl;
		}
	}

	//////////////////////////////////////////////////
	// destructor
	auto dtor = typeInfo.Dtor();
	if (dtor)
	{
		dtor.Invoke(testA);
	}

	/////////////////////////////////////////////////
	// base
	testA.mTTT = 33;

	auto base = typeInfo.Base();
	if (base)
	{
		auto varInfo = base.GetType().Var(UID_HASH("mTTT"));
		if (varInfo)
		{
			auto value = varInfo.Get(testA);
			if (value) {
				std::cout << "TestC::mTTT:" << *value.TryCast<int>() << std::endl;
			}
		}
	}
}

#endif