#include "engineWin32.h"
#include "client\app\systemEvent.h"
#include "client\app\win32\gameWindowWin32.h"
#include "core\platform\platform.h"
#include "core\plugin\pluginManager.h"
#include "core\plugin\modulePulgin.h"
#include "core\filesystem\filesystem_physfs.h"
#include "core\jobsystem\jobsystem.h"
#include "resource\resourceManager.h"

namespace Cjing3D::Win32
{
	struct EngineWin32Impl
	{
		SharedPtr<GameWindowWin32> mGameWindowWin32 = nullptr;
		ScopedConnection mSystemConnection;
		SharedPtr<EventQueue> mSystemEventQueue = nullptr;

		BaseFileSystem* mFileSystem = nullptr;
	};

	EngineWin32::EngineWin32(SharedPtr<GameWindowWin32> gameWindow, InitConfig& config) :
		Engine(gameWindow, config)
	{
		mImpl = CJING_NEW(EngineWin32Impl);
		mImpl->mGameWindowWin32 = gameWindow;
	}

	EngineWin32::~EngineWin32()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void EngineWin32::Initialize()
	{
		JobSystem::Initialize(4, JobSystem::MAX_FIBER_COUNT, JobSystem::FIBER_STACK_SIZE);

		PluginManager::Initialize();

#ifdef CJING_PLUGINS
		const char* plugins[] = { CJING_PLUGINS };
		Span<const char*> pluginSpan = Span(plugins);
		for (const char* plugin : pluginSpan) {
			PluginManager::LoadPlugin(plugin);
		}
#endif

		BaseFileSystem* filesystem = nullptr;
		if (mInitConfig.mPackPath != nullptr) {
			// create pack filesystem
		}
		else if (mInitConfig.mWorkPath != nullptr) {
			filesystem = CJING_NEW(FileSystemPhysfs)(mInitConfig.mWorkPath);
		}
		else
		{	
			// 以当前目录创建filesystem
			MaxPathString dirPath;
			Platform::GetCurrentDir(dirPath.toSpan());
			filesystem = CJING_NEW(FileSystemPhysfs)(dirPath.c_str());
		}
		mImpl->mFileSystem = filesystem;
		ResourceManager::Initialize(filesystem);

		// load custom plugins
		for (const char* plugin : mInitConfig.mPlugins) {
			PluginManager::LoadPlugin(plugin);
		}

		// acquire all moduler plugins
		DynamicArray<Plugin*> tempPlugins;
		PluginManager::GetPlugins<ModulerPlugin>(tempPlugins);
		for (auto plugin : tempPlugins) {
			mModulerPlugins.push(reinterpret_cast<ModulerPlugin*>(plugin));
		}

		// init all plugins
		for (ModulerPlugin* plugin : mModulerPlugins) {
			plugin->Initialize();
		}
	}

	void EngineWin32::Uninitialize()
	{
		for (ModulerPlugin* plugin : mModulerPlugins) {
			plugin->Initialize();
		}

		// uninit resource manager
		ResourceManager::Uninitialize();

		CJING_SAFE_DELETE(mImpl->mFileSystem);

		// uninit plugin manager
		PluginManager::Uninitialize();

		// uninit jobsystem
		JobSystem::Uninitialize();
	}

	void EngineWin32::Update()
	{
	}

	void EngineWin32::SetSystemEventQueue(const SharedPtr<EventQueue>& eventQueue)
	{
		mImpl->mSystemEventQueue = eventQueue;
		mImpl->mSystemConnection = eventQueue->Connect([this](const Event& event) {
			HandleSystemMessage(event);
		});
	}

	void EngineWin32::HandleSystemMessage(const Event& systemEvent)
	{
		if (systemEvent.Is<WindowCloseEvent>())
		{
			mImpl->mGameWindowWin32->SetIsExiting(true);
		}
		else if (systemEvent.Is<InputTextEvent>())
		{
			const InputTextEvent* event = systemEvent.As<InputTextEvent>();

		}
		else if (systemEvent.Is<RAWMOUSE>())
		{
			const RAWMOUSE* event = systemEvent.As<RAWMOUSE>();

		}
		else if (systemEvent.Is<RAWKEYBOARD>())
		{
			const RAWKEYBOARD* event = systemEvent.As<RAWKEYBOARD>();

		}
		else if (systemEvent.Is<ViewResizeEvent>())
		{
			const ViewResizeEvent* event = systemEvent.As<ViewResizeEvent>();

		}
	}

	void EngineWin32::DoSystemEvents()
	{
		mImpl->mSystemEventQueue->FireEvents();
	}

	BaseFileSystem* EngineWin32::GetFileSystem()
	{
		return mImpl->mFileSystem;
	}
}