#pragma once

#include "core\common\common.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace Cjing3D
{
	namespace GPU
	{
		class CommandList;
	}

	namespace ImGuiRHI
	{
		namespace Manager
		{
			void Initialize(ImGuiConfigFlags configFlags);
			void Uninitialize();
			bool IsInitialized();
			void Render(GPU::CommandList& cmd);

			void BeginFrame();
			void EndFrame();
		};
	}
}