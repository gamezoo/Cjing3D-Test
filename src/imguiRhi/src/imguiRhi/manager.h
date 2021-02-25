#pragma once

#include "core\common\common.h"
#include "core\platform\platform.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace Cjing3D
{
	namespace GPU
	{
		class CommandList;
	}

	class InputManager;

	namespace ImGuiRHI
	{
		namespace Manager
		{
			void Initialize(ImGuiConfigFlags configFlags, Platform::WindowType mainWindow);
			void Uninitialize();
			bool IsInitialized();
			void Render(GPU::CommandList& cmd);
			
			void BeginFrame(InputManager& input, F32 width, F32 height, F32 deltaTime);
			void EndFrame();
		};
	}
}