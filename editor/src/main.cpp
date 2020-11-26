
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

#include "core\plugin\plugin.h"
#include "core\plugin\pluginManager.h"
#include "core\container\span.h"
#include "core\jobsystem\jobsystem.h"
#include "resource\resourceManager.h"
#include "resource\resRef.h"

class TestRes : public Resource
{
public:
	DECLARE_RESOURCE(TestRes, "Test")

	TestRes() = default;

	String mText;
};
using TestResRef = ResRef<TestRes>;

class TestResFactory : public ResourceFactory
{
public:
	virtual Resource* CreateResource()
	{
		TestRes* testRes = CJING_NEW(TestRes)();
		return testRes;
	}

	virtual bool LoadResource(Resource* resource, const char* name, File& file)
	{
		if (!file) {
			return false;
		}

		TestRes* res = reinterpret_cast<TestRes*>(resource);
		if (res == nullptr) {
			return false;
		}

		res->mText.resize(file.Size());
		file.Read(res->mText.data(), file.Size());

		return true;
	}

	virtual bool DestroyResource(Resource* resource)
	{
		if (resource == nullptr) {
			return false;
		}

		TestRes* res = reinterpret_cast<TestRes*>(resource);
		if (res == nullptr) {
			return false;
		}

		res->mText.clear();
		CJING_DELETE(res);
		return true;
	}

	virtual bool IsNeedConvert()const
	{
		return true;
	}
};
DEFINE_RESOURCE(TestRes, "Test")


int main()
{
	PluginManager::Initialize();

#ifdef CJING_PLUGINS
	const char* plugins[] = { CJING_PLUGINS };
	Span<const char*> pluginSpan = Span(plugins);
	for (const char* plugin : pluginSpan)
	{
		std::cout << plugin << std::endl;
		PluginManager::LoadPlugin(plugin);
	}
#endif
	JobSystem::ScopedManager scoped(4, JobSystem::MAX_FIBER_COUNT, JobSystem::FIBER_STACK_SIZE);

	MaxPathString dirPath;
	Platform::GetCurrentDir(dirPath.toSpan());
	ResourceManager::Initialize(dirPath.c_str());

	TestRes::RegisterFactory();
	{
		TestResRef ref = ResourceManager::LoadResourceImmediate<TestRes>("Test.txt");
		//ResourceManager::WaitForResource(ref);
		std::cout << "Resource Ref Test" << std::endl;
	}
	TestRes::UnregisterFactory();

	ResourceManager::Uninitialize();
	PluginManager::Uninitialize();
	return 0;
}