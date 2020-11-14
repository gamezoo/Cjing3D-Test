
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

#include "resource\resource.h"
#include "resource\resourceManager.h"
#include "core\memory\memory.h"

class TestResFactory : public ResourceFactory
{
public:
	virtual Resource* CreateResource()
	{
		return nullptr;
	}

	virtual Resource* LoadResource(const Path& path)
	{
		return nullptr;
	}

	virtual void DestroyResource(Resource& resource)
	{
	
	}
};

class TestRes : public Resource
{
public:
	DECLARE_RESOURCE(TestRes, "Test")

	TestRes() = default;
};

DEFINE_RESOURCE(TestRes, "Test")


int main()
{
	ResourceManager::Initialize();
	TestRes::RegisterFactory();


	TestRes::UnregisterFactory();
	ResourceManager::Uninitialize();

	std::cout << "Hello world!" << std::endl;
	return 0;
}