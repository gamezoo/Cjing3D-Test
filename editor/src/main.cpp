
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
	int mValue = 0;
	int mAge = 12;
};

int main()
{
	Reflection::Reflect<TestA>(UID_HASH("TestA"))
		.AddVar<&TestA::mValue>(UID_HASH("mValue"));

	std::cout << "Hello world!" << std::endl;
	return 0;
}