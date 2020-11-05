
#include <windows.h>
#include <iostream>

#include "client\app\win32\gameAppWin32.h"

using namespace Cjing3D;

//int WINAPI WinMain(
//	_In_ HINSTANCE hInstance,
//	_In_opt_ HINSTANCE hPrevInstance,
//	_In_ LPSTR lpCmdLine,
//	_In_ int nShowCmd)
//{
//	Win32::GameAppWin32 gameApp;
//	gameApp.SetInstance(hInstance);
//	gameApp.SetAssetPath(".", "Assets");
//	gameApp.SetScreenSize({ 1280, 720 });
//	gameApp.SetTitleName(String("Cjing3D ") + CjingVersion::GetVersionString());
//	gameApp.Run();
//
//	std::cout << "Hello world!" << std::endl;
//	return 0;
//}

#include "core\helper\reflection.h"

class TestA
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

int main()
{
	Reflection::Reflect<TestA>(UID_HASH("TestA"))
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

	std::cout << "Hello world!" << std::endl;
	return 0;
}