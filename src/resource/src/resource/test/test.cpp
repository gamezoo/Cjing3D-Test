//#define CJING_TEST_RESOURCE
#ifdef CJING_TEST_RESOURCE

#include "resource\resRef.h"
#include "resource\resourceManager.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"
#include "core\filesystem\filesystem.h"
#include "core\filesystem\filesystem_physfs.h"
#include "core\concurrency\jobsystem.h"
#include "core\helper\timer.h"
#include "core\plugin\pluginManager.h"

#define CATCH_CONFIG_RUNNER
#include "catch\catch.hpp"

using namespace Cjing3D;

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

TEST_CASE("load resource (no converter)", "[resource]")
{
	TestRes::RegisterFactory();
	{
		TestResRef ref1 = ResourceManager::LoadResource<TestRes>("Test.txt");
		TestResRef ref2 = ResourceManager::LoadResource<TestRes>("Test.txt");
		TestResRef ref3 = ResourceManager::LoadResource<TestRes>("Test.txt");
		ref1.WaitUntilLoaded();
		//ResourceManager::WaitForResource(ref);
		std::cout << "Resource Ref Test" << std::endl;
	}
	TestRes::UnregisterFactory();
}

TEST_CASE("load resource (converter)", "[resource]")
{
	TestRes::RegisterFactory();
	{
		TestResRef ref = ResourceManager::LoadResourceImmediate<TestRes>("Test.txt");
		//ResourceManager::WaitForResource(ref);
		std::cout << "Resource Ref Test" << std::endl;
	}
	TestRes::UnregisterFactory();
}

TEST_CASE("file io read", "[resource]")
{
	ResourceManager::AsyncHandle handle;
	char* buffer = nullptr;
	size_t size = 0;
	ResourceManager::ReadFileData("Test.txt", buffer, size, &handle);

	F64 startTime = Timer::GetAbsoluteTime();
	do {
		Logger::Info("Wait for reading. time:%.2f remaining:%d", Timer::GetAbsoluteTime() - startTime, handle.mRemaining);
	} while (!handle.IsComplete());

	CJING_SAFE_DELETE_ARR(buffer, size);
}

TEST_CASE("file io write", "[resource]")
{
	String ouputText = "I wanna to be a guy!";
	ResourceManager::AsyncHandle handle;
	ResourceManager::WriteFileData("Test.txt", ouputText.data(), ouputText.length(), &handle);

	F64 startTime = Timer::GetAbsoluteTime();
	do {
		Logger::Info("Wait for writing. time:%.2f remaining:%d", Timer::GetAbsoluteTime() - startTime, handle.mRemaining);
	} while (!handle.IsComplete());
}

int main(int argc, char* argv[])
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

	auto ret = Catch::Session().run(argc, argv);

	ResourceManager::Uninitialize();
	PluginManager::Uninitialize();

	return ret;
}

#endif