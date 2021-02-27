#include "imguiEx.h"

using namespace ImGui;

namespace Cjing3D
{
	void Cjing3D::ImGuiEx::VSplitter(const char* name, ImVec2& size)
	{
		ImVec2 screen_pos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton(name, ImVec2(3, -1));
		ImVec2 end_pos(
			screen_pos.x + ImGui::GetItemRectSize().x,
			screen_pos.y + ImGui::GetItemRectSize().y);

		ImGuiWindow* win = ImGui::GetCurrentWindow();
		ImVec4* colors = ImGui::GetStyle().Colors;
		ImU32 color = ImGui::GetColorU32(ImGui::IsItemActive() || ImGui::IsItemHovered() ? 
			colors[ImGuiCol_ButtonActive] : colors[ImGuiCol_Button]);
		win->DrawList->AddRectFilled(screen_pos, end_pos, color);
	}
}


