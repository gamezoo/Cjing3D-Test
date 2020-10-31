#include "engineWin32.h"
#include "core\platform\platform.h"
#include "client\app\systemEvent.h"
#include "client\app\win32\gameWindowWin32.h"

namespace Cjing3D::Win32
{
	EngineWin32::EngineWin32(SharedPtr<GameWindowWin32> gameWindow, PresentConfig& config) :
		Engine(gameWindow, config),
		mGameWindowWin32(gameWindow)
	{
	}

	EngineWin32::~EngineWin32()
	{
	}

	void EngineWin32::Initialize()
	{
	}

	void EngineWin32::Uninitialize()
	{
	}

	void EngineWin32::SetAssetPath(const String& path, const String& name)
	{
		mAssetPath = path;
		mAssetName = name;
	}

	void EngineWin32::SetSystemEventQueue(const SharedPtr<EventQueue>& eventQueue)
	{
		mSystemEventQueue = eventQueue;
		mSystemConnection = eventQueue->Connect([this](const Event& event) {
			HandleSystemMessage(event);
		});
	}

	void EngineWin32::HandleSystemMessage(const Event& systemEvent)
	{
		if (systemEvent.Is<WindowCloseEvent>())
		{
			SetIsExiting(true);
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
		mSystemEventQueue->FireEvents();
	}
}