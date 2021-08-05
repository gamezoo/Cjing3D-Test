#include "imguiEx.h"

using namespace ImGui;

namespace Cjing3D
{
	void ImGuiEx::VLeftLabel(const char* text)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		const ImVec2 lineStart = ImGui::GetCursorScreenPos();
		const ImGuiStyle& style = ImGui::GetStyle();
		F32 fullWidth = ImGui::GetContentRegionAvail().x;
		F32 leftPercent = 0.4f;

		ImVec2 textSize = ImGui::CalcTextSize(text);
		ImRect textRect;
		textRect.Min = ImGui::GetCursorScreenPos();
		textRect.Max = textRect.Min;
		textRect.Max.x += fullWidth * leftPercent;
		textRect.Max.y += textSize.y;
		ImGui::SetCursorScreenPos(textRect.Min);

		ImGui::AlignTextToFramePadding();
		textRect.Min.y += window->DC.CurrLineTextBaseOffset;
		textRect.Max.y += window->DC.CurrLineTextBaseOffset;

		ImGui::ItemSize(textRect);
		if (ImGui::ItemAdd(textRect, window->GetID(text)))
		{
			ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), textRect.Min, textRect.Max, textRect.Max.x, textRect.Max.x, text, nullptr, &textSize);
			if (textRect.GetWidth() < textSize.x && ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", text);
			}
		}
		ImGui::SetCursorScreenPos(ImVec2{ textRect.Max.x, textRect.Max.y - (textSize.y + window->DC.CurrLineTextBaseOffset) });
		ImGui::SameLine();
		ImGui::SetNextItemWidth(fullWidth * (1.0f - leftPercent));
	}

	void Cjing3D::ImGuiEx::VSplitter(const char* name, ImVec2& size)
	{
		ImVec2 screenPos = ImGui::GetCursorScreenPos();
		ImGui::InvisibleButton(name, ImVec2(3, -1));
		ImVec2 endPos(
			screenPos.x + ImGui::GetItemRectSize().x,
			screenPos.y + ImGui::GetItemRectSize().y);

		ImGuiWindow* win = ImGui::GetCurrentWindow();
		ImVec4* colors = ImGui::GetStyle().Colors;
		ImU32 color = ImGui::GetColorU32(ImGui::IsItemActive() || ImGui::IsItemHovered() ? colors[ImGuiCol_ButtonActive] : colors[ImGuiCol_Button]);
		win->DrawList->AddRectFilled(screenPos, endPos, color);
	}

	void ImGuiEx::Rect(F32 w, F32 h, ImU32 color)
	{
		ImGuiWindow* win = GetCurrentWindow();
		ImVec2 screenPos = GetCursorScreenPos();
		ImVec2 endPos(screenPos.x + w, screenPos.y + h);
		ImRect rect(screenPos, endPos);
		ItemSize(rect);
		if (!ItemAdd(rect, 0)) {
			return;
		}
		win->DrawList->AddRectFilled(screenPos, endPos, color);
	}
}


