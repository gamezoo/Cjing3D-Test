#include "engineWin32.h"
#include "core\platform\events.h"
#include "client\app\win32\gameWindowWin32.h"
#include "client\app\win32\input\inputSystemWin32.h"
#include "core\platform\platform.h"
#include "core\plugin\pluginManager.h"
#include "core\plugin\modulePulgin.h"
#include "core\filesystem\filesystem_physfs.h"
#include "core\concurrency\jobsystem.h"
#include "core\scripts\luaContext.h"
#include "core\helper\profiler.h"
#include "core\helper\buildConfig.h"
#include "resource\resourceManager.h"
#include "renderer\renderer.h"
#include "gpu\device.h"

namespace Cjing3D::Win32
{
	struct EngineWin32Impl
	{
		SharedPtr<GameWindowWin32> mGameWindowWin32 = nullptr;
		ScopedConnection mSystemConnection;
		SharedPtr<EventQueue> mSystemEventQueue = nullptr;

		BaseFileSystem* mFileSystem = nullptr;
		Win32::InputManagerWin32* mInputSystem = nullptr;
		LuaContext* mLuaContext = nullptr;
		File mLogFile;
		FileLoggerSink mFileLoggerSink;
	};

	EngineWin32::EngineWin32(SharedPtr<GameWindowWin32> gameWindow, InitConfig& config) :
		Engine(gameWindow, config)
	{
		mImpl = CJING_NEW(EngineWin32Impl);
		mImpl->mGameWindowWin32 = gameWindow;

		mImpl->mLogFile = File("Cjing3D.log", FileFlags::DEFAULT_WRITE);
		if (mImpl->mLogFile.IsValid()) {
			mImpl->mFileLoggerSink.SetLogFile(mImpl->mLogFile);
		}
		Logger::RegisterSink(mImpl->mFileLoggerSink);
	}

	EngineWin32::~EngineWin32()
	{
		Logger::UnregisterSink(mImpl->mFileLoggerSink);

		CJING_SAFE_DELETE(mImpl);
	}

	void EngineWin32::Initialize()
	{
		Logger::Info("Initializing engine...");

		// init platform
		Platform::Initialize();

		// init profiler
		Profiler::Initialize();
		Profiler::SetCurrentThreadName("MainThread");

		// init jobsystme
		JobSystem::Initialize(4, JobSystem::MAX_FIBER_COUNT, JobSystem::FIBER_STACK_SIZE);

		// init plugin manager
		PluginManager::Initialize(*this);

#ifdef CJING_PLUGINS
		const char* plugins[] = { CJING_PLUGINS };
		Span<const char*> pluginSpan = Span(plugins);
		for (const char* plugin : pluginSpan) {
			PluginManager::LoadPlugin(plugin);
		}
#endif
		// init lua context
		mImpl->mLuaContext = CJING_NEW(LuaContext)(*this);
		mImpl->mLuaContext->Initialize();

		// init filesystem
		BaseFileSystem* filesystem = nullptr;
		if (mInitConfig.mPackPath != nullptr)
		{
			// create pack filesystem
			// todo...
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

		// init build config (filesystem must is initialized, must before resourceManager)
		BuildConfig::Initialize(filesystem);

		// init resource manager
		ResourceManager::Initialize(filesystem);

		// init input system
		mImpl->mInputSystem = CJING_NEW(Win32::InputManagerWin32);
		mImpl->mInputSystem->Initialize(*mImpl->mGameWindowWin32);

		// init renderer
		GPU::GPUSetupParams gpuSetupParams;
		gpuSetupParams.mWindow = mImpl->mGameWindowWin32->GetHwnd();
		gpuSetupParams.mIsFullscreen = mInitConfig.mIsFullScreen;
		Renderer::Initialize(gpuSetupParams, mInitConfig.mIsApp ? true : false);

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

		Logger::Info("Engine initialized");
	}

	void EngineWin32::Uninitialize()
	{
		for (ModulerPlugin* plugin : mModulerPlugins) {
			plugin->Initialize();
		}

		// unit renderer
		Renderer::Uninitialize();

		// uninit input system
		mImpl->mInputSystem->Uninitialize();
		CJING_SAFE_DELETE(mImpl->mInputSystem);

		// uninit resource manager
		ResourceManager::Uninitialize();

		// uninit filesystem
		CJING_SAFE_DELETE(mImpl->mFileSystem);

		// uninit lua context
		mImpl->mLuaContext->Uninitialize();
		CJING_SAFE_DELETE(mImpl->mLuaContext);

		// uninit plugin manager
		PluginManager::Uninitialize();

		// uninit jobsystem
		JobSystem::Uninitialize();

		// uninit profiler
		Profiler::Uninitilize();

		Logger::Info("Engine uninitialized");
	}

	void EngineWin32::Update(Universe& universe, F32 dt)
	{
		// update input system
		mImpl->mInputSystem->Update(dt);

		// update universe
		universe.Update(dt);

		// update scenes
		auto& scenes = universe.GetScenes();
		for (auto& scene : scenes) {
			scene->Update(dt);
		}

		for (auto& scene : scenes) {
			scene->LateUpdate(dt);
		}

		// update all plugins
		for (ModulerPlugin* plugin : mModulerPlugins) {
			plugin->Update(dt);
		}
	}

	void EngineWin32::FixedUpdate()
	{
	}

	void EngineWin32::RequestExit()
	{
		mImpl->mGameWindowWin32->SetIsExiting(true);
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
			mImpl->mInputSystem->ProcessTextInput(*event);

		}
		else if (systemEvent.Is<RAWMOUSE>())
		{
			const RAWMOUSE* event = systemEvent.As<RAWMOUSE>();
			mImpl->mInputSystem->ProcessMouseEvent(*event);
		}
		else if (systemEvent.Is<RAWKEYBOARD>())
		{
			const RAWKEYBOARD* event = systemEvent.As<RAWKEYBOARD>();
			mImpl->mInputSystem->ProcessKeyboardEvent(*event);
		}
		else if (systemEvent.Is<ViewResizeEvent>())
		{
			const ViewResizeEvent* event = systemEvent.As<ViewResizeEvent>();
		}
	}

	void EngineWin32::DoSystemEvents()
	{
		mImpl->mInputSystem->Reset();
		mImpl->mSystemEventQueue->FireEvents();
	}

	BaseFileSystem* EngineWin32::GetFileSystem()
	{
		return mImpl->mFileSystem;
	}

	InputManager* EngineWin32::GetInputManager()
	{
		return mImpl->mInputSystem;
	}

	EventQueue* EngineWin32::GetEventQueue()
	{
		return mImpl->mSystemEventQueue.get();
	}
}