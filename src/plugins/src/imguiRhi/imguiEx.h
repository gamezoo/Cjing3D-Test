#pragma once

#include "core\common\common.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace Cjing3D
{
namespace ImGuiEx
{
	void VLeftLabel(const char* text);
	void VSplitter(const char* name, ImVec2& size);
	void Rect(F32 w, F32 h, ImU32 color);
}
}