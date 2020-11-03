
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

	TestA(const TestA& rhs)
	{
		std::cout << "TestA Copy Constructor" << std::endl;
	}

	TestA(TestA&& rhs)
	{
		std::cout << "TestA Move Constructor" << std::endl;
	}

	int mValue = 0;
	int mAge = 12;
	int mArray[10];

	static int mStaticValue;
};

int TestA::mStaticValue = 10;

int main()
{
	Reflection::Reflect<TestA>(UID_HASH("TestA"))
		.AddVar<&TestA::mStaticValue>(UID_HASH("mStaticValue"));

	TestA::mStaticValue = 12;

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
	std::cout << "Hello world!" << std::endl;
	return 0;
}